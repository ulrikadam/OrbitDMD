/* OrbitDMD - mise au format du panneau des GIF trop grandes (page GIF)
   Le navigateur decode la GIF, la reduit (pixels les plus proches : le pixel art reste net),
   puis la reencode. Les GIF qui tiennent deja sur le panneau ne sont pas touchees. */
"use strict";

const GifScale = {
	/* largeur et hauteur lues dans l'en-tete (10 premiers octets) */
	size (bytes) {
		if (bytes.length < 10 || String.fromCharCode(bytes[0], bytes[1], bytes[2]) !== "GIF") return null;
		return { w: bytes[6] | bytes[7] << 8, h: bytes[8] | bytes[9] << 8 };
	},

	/* --- decodage : images completes (RGBA) apres chaque etape de l'animation --- */
	decode (bytes) {
		let p = 13;
		const W = bytes[6] | bytes[7] << 8, H = bytes[8] | bytes[9] << 8, flags = bytes[10];
		const readTable = n => { const t = bytes.subarray(p, p + n * 3); p += n * 3; return t; };
		const global = flags & 0x80 ? readTable(1 << ((flags & 7) + 1)) : null;
		const canvas = new Uint8ClampedArray(W * H * 4);	/* fond transparent */
		const frames = [];
		let gce = { disposal: 0, transparent: -1, delay: 10 };

		while (p < bytes.length) {
			const b = bytes[p++];
			if (b === 0x3B) break;
			if (b === 0x21) {
				const label = bytes[p++];
				if (label === 0xF9 && bytes[p] >= 4) {
					const f = bytes[p + 1];
					gce = { disposal: f >> 2 & 7, transparent: f & 1 ? bytes[p + 4] : -1, delay: bytes[p + 2] | bytes[p + 3] << 8 };
				}
				while (bytes[p]) p += bytes[p] + 1;	/* sous-blocs */
				p++;
				continue;
			}
			if (b !== 0x2C) throw new Error("GIF illisible");

			const x0 = bytes[p] | bytes[p + 1] << 8, y0 = bytes[p + 2] | bytes[p + 3] << 8;
			const fw = bytes[p + 4] | bytes[p + 5] << 8, fh = bytes[p + 6] | bytes[p + 7] << 8, ff = bytes[p + 8];
			p += 9;
			const table = ff & 0x80 ? readTable(1 << ((ff & 7) + 1)) : global;
			if (!table) throw new Error("GIF sans palette");
			const minCode = bytes[p++];
			/* donnees : sous-blocs mis bout a bout */
			let len = 0, q = p;
			while (bytes[q]) { len += bytes[q]; q += bytes[q] + 1; }
			const data = new Uint8Array(len);
			for (let o = 0; bytes[p]; p += bytes[p] + 1) { data.set(bytes.subarray(p + 1, p + 1 + bytes[p]), o); o += bytes[p]; }
			p++;
			const pixels = GifScale.lzwDecode(data, minCode, fw * fh);

			const previous = gce.disposal === 3 ? canvas.slice() : null;
			/* lignes entrelacees : ordre 0,8,16.. 4,12.. 2,6.. 1,3.. */
			const rows = [];
			if (ff & 0x40) { for (const [s, d] of [[0, 8], [4, 8], [2, 4], [1, 2]]) for (let y = s; y < fh; y += d) rows.push(y); }
			else for (let y = 0; y < fh; y++) rows.push(y);
			for (let r = 0; r < fh; r++) {
				const y = y0 + rows[r];
				if (y >= H) continue;
				for (let x = 0; x < fw; x++) {
					const idx = pixels[r * fw + x];
					if (idx === gce.transparent || x0 + x >= W) continue;
					const o = (y * W + x0 + x) * 4;
					canvas[o] = table[idx * 3]; canvas[o + 1] = table[idx * 3 + 1]; canvas[o + 2] = table[idx * 3 + 2]; canvas[o + 3] = 255;
				}
			}
			frames.push({ rgba: canvas.slice(), delay: gce.delay });

			if (gce.disposal === 2) {
				for (let y = y0; y < Math.min(H, y0 + fh); y++) canvas.fill(0, (y * W + x0) * 4, (y * W + Math.min(W, x0 + fw)) * 4);
			} else if (gce.disposal === 3 && previous) canvas.set(previous);
			gce = { disposal: 0, transparent: -1, delay: 10 };
		}
		if (!frames.length) throw new Error("GIF sans image");
		return { w: W, h: H, frames };
	},

	lzwDecode (data, minCode, count) {
		const out = new Uint8Array(count);
		const clear = 1 << minCode, end = clear + 1;
		const prefix = new Int16Array(4096), suffix = new Uint8Array(4096), first = new Uint8Array(4096);
		const stack = new Uint8Array(4097);
		for (let i = 0; i < clear; i++) { prefix[i] = -1; suffix[i] = i; first[i] = i; }
		let size = minCode + 1, next = end + 1, old = -1, bits = 0, acc = 0, n = 0, pos = 0;
		while (n < count) {
			while (bits < size && pos < data.length) { acc |= data[pos++] << bits; bits += 8; }
			if (bits < size) break;
			const code = acc & ((1 << size) - 1);
			acc >>>= size; bits -= size;
			if (code === clear) { size = minCode + 1; next = end + 1; old = -1; continue; }
			if (code === end) break;
			let c = code, sp = 0;
			if (old === -1) { if (code >= clear) break; out[n++] = code; old = code; continue; }
			if (code >= next) { stack[sp++] = first[old]; c = old; }	/* code pas encore connu */
			while (c >= clear) { stack[sp++] = suffix[c]; c = prefix[c]; }
			stack[sp++] = c;
			if (next < 4096) { prefix[next] = old; suffix[next] = c; first[next] = first[old]; next++; }
			while (sp && n < count) out[n++] = stack[--sp];
			if (next === 1 << size && size < 12) size++;
			old = code;
		}
		return out;
	},

	/* --- reduction (pixels les plus proches), sur fond noir --- */
	scale (gif, maxW, maxH) {
		const s = Math.min(maxW / gif.w, maxH / gif.h);
		const w = Math.max(1, Math.round(gif.w * s)), h = Math.max(1, Math.round(gif.h * s));
		const frames = gif.frames.map(f => {
			const out = new Uint8Array(w * h * 3);
			for (let y = 0; y < h; y++) {
				const sy = Math.min(gif.h - 1, Math.floor((y + 0.5) * gif.h / h));
				for (let x = 0; x < w; x++) {
					const sx = Math.min(gif.w - 1, Math.floor((x + 0.5) * gif.w / w));
					const i = (sy * gif.w + sx) * 4, o = (y * w + x) * 3;
					const a = f.rgba[i + 3] / 255;		/* transparent -> noir, comme sur le panneau */
					out[o] = f.rgba[i] * a; out[o + 1] = f.rgba[i + 1] * a; out[o + 2] = f.rgba[i + 2] * a;
				}
			}
			return { rgb: out, delay: f.delay };
		});
		return { w, h, frames };
	},

	/* --- encodage : une palette commune (256 couleurs au plus), image complete a chaque etape --- */
	encode (img) {
		/* couleurs les plus utilisees ; les autres prennent la plus proche */
		const counts = new Map();
		for (const f of img.frames) for (let i = 0; i < f.rgb.length; i += 3) {
			const c = f.rgb[i] << 16 | f.rgb[i + 1] << 8 | f.rgb[i + 2];
			counts.set(c, (counts.get(c) || 0) + 1);
		}
		const palette = [...counts.keys()].sort((a, b) => counts.get(b) - counts.get(a)).slice(0, 256);
		const index = new Map(palette.map((c, i) => [c, i]));
		const nearest = c => {
			let best = 0, bd = Infinity;
			const r = c >> 16, g = c >> 8 & 255, b = c & 255;
			palette.forEach((p, i) => {
				const d = (r - (p >> 16)) ** 2 + (g - (p >> 8 & 255)) ** 2 + (b - (p & 255)) ** 2;
				if (d < bd) { bd = d; best = i; }
			});
			index.set(c, best);
			return best;
		};
		let bits = 1;
		while (1 << bits < palette.length) bits++;
		const out = [];
		const u16 = v => out.push(v & 255, v >> 8 & 255);
		for (const ch of "GIF89a") out.push(ch.charCodeAt(0));
		u16(img.w); u16(img.h);
		out.push(0x80 | 0x70 | (bits - 1), 0, 0);
		for (let i = 0; i < 1 << bits; i++) { const c = palette[i] || 0; out.push(c >> 16, c >> 8 & 255, c & 255); }
		if (img.frames.length > 1) out.push(0x21, 0xFF, 11, ..."NETSCAPE2.0".split("").map(ch => ch.charCodeAt(0)), 3, 1, 0, 0, 0);
		for (const f of img.frames) {
			out.push(0x21, 0xF9, 4, 0x04, f.delay & 255, f.delay >> 8 & 255, 0, 0);	/* garder l'image */
			out.push(0x2C); u16(0); u16(0); u16(img.w); u16(img.h); out.push(0);
			const px = new Uint8Array(img.w * img.h);
			for (let i = 0, j = 0; i < f.rgb.length; i += 3, j++) {
				const c = f.rgb[i] << 16 | f.rgb[i + 1] << 8 | f.rgb[i + 2];
				px[j] = index.has(c) ? index.get(c) : nearest(c);
			}
			const minCode = Math.max(2, bits);
			out.push(minCode);
			const data = GifScale.lzwEncode(px, minCode);
			for (let i = 0; i < data.length; i += 255) { const n = Math.min(255, data.length - i); out.push(n); for (let k = 0; k < n; k++) out.push(data[i + k]); }
			out.push(0);
		}
		out.push(0x3B);
		return new Uint8Array(out);
	},

	lzwEncode (px, minCode) {
		const clear = 1 << minCode, end = clear + 1;
		const out = [];
		let size = minCode + 1, next = end + 1, acc = 0, bits = 0;
		/* comme le compress classique : apres chaque code ecrit, un bit de plus des que
		   le prochain numero libre ne tient plus dans la taille actuelle */
		const emit = code => {
			acc |= code << bits; bits += size;
			while (bits >= 8) { out.push(acc & 255); acc >>>= 8; bits -= 8; }
			if (next > (1 << size) - 1 && size < 12) size++;
		};
		let dict = new Map();
		emit(clear);
		let cur = px[0];
		for (let i = 1; i < px.length; i++) {
			const k = px[i], key = cur << 8 | k;
			const found = dict.get(key);
			if (found !== undefined) { cur = found; continue; }
			emit(cur);
			if (next < 4096) {
				dict.set(key, next++);
			} else {
				emit(clear);			/* table pleine : on repart de zero */
				dict = new Map(); size = minCode + 1; next = end + 1;
			}
			cur = k;
		}
		emit(cur);
		emit(end);
		if (bits > 0) out.push(acc & 255);
		return out;
	},

	/* GIF plus grande que le panneau : nouvelle GIF a sa taille ; sinon null (envoyer telle quelle) */
	async fit (file, maxW, maxH) {
		const bytes = new Uint8Array(await file.arrayBuffer());
		const size = GifScale.size(bytes);
		if (!size || (size.w <= maxW && size.h <= maxH)) return null;
		const small = GifScale.scale(GifScale.decode(bytes), maxW, maxH);
		return new File([GifScale.encode(small)], file.name, { type: "image/gif" });
	}
};

if (typeof module !== "undefined") module.exports = GifScale;	/* essais avec Node */
