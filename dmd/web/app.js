/* OrbitDMD - fonctions communes a toutes les pages */
"use strict";

const $ = (sel, el = document) => el.querySelector(sel);

/* --- langue de l'interface ---
   Les pages sont ecrites en francais. Pour une autre langue, lang-xx.js (genere depuis
   outils/traductions/web.txt par outils/langues.py) appelle I18N.start(dictionnaire) :
   chaque texte de la page (et chaque texte ajoute ensuite par les scripts) est remplace
   par sa traduction. Cle "{}" : partie variable ("il y a {} min") ; dans la traduction,
   "{}" reprend les parties dans l'ordre, "{1}", "{2}"... dans un autre ordre. */
const LANGS = { fr: "Français", en: "English", nl: "Nederlands", es: "Español", ru: "Русский", de: "Deutsch",
	it: "Italiano", pt: "Português", pl: "Polski", uk: "Українська", tr: "Türkçe", cs: "Čeština" };
const LANG_FILES = { en: "/lang-en.js", nl: "/lang-nl.js", es: "/lang-es.js", ru: "/lang-ru.js", de: "/lang-de.js",
	it: "/lang-it.js", pt: "/lang-pt.js", pl: "/lang-pl.js", uk: "/lang-uk.js", tr: "/lang-tr.js", cs: "/lang-cs.js" };
const I18N = {
	lang: "fr",
	dict: null,
	tpls: [],
	SKIP: { SCRIPT: 1, STYLE: 1, TEXTAREA: 1, CODE: 1, PRE: 1 },
	ATTRS: ["placeholder", "title", "aria-label", "alt"],

	choose () {
		let l = "";
		try { l = localStorage.getItem("orbitdmd_langue") || ""; } catch (e) {}
		if (!LANGS[l]) {
			const wanted = (navigator.languages && navigator.languages.length ? navigator.languages : [navigator.language || "fr"])
				.map(x => String(x).slice(0, 2).toLowerCase());
			l = wanted.find(x => LANGS[x]) || "en";
		}
		this.lang = l;
		document.documentElement.lang = l;
		if (LANG_FILES[l]) document.write('<script src="' + LANG_FILES[l] + '"><\/script>');
	},
	set (l) {
		try { localStorage.setItem("orbitdmd_langue", l); } catch (e) {}
		location.reload();
	},

	/* dictionnaire charge : traduit la page, puis tout ce qui change */
	start (dict) {
		this.dict = dict;
		const esc = s => s.replace(/[.*+?^$()|[\]\\]/g, "\\$&");
		for (const k in dict) {
			if (k.includes("{}")) this.tpls.push([new RegExp("^" + k.split("{}").map(esc).join("([\\s\\S]+?)") + "$"), dict[k]]);
		}
		this.tpls.sort((a, b) => b[0].source.length - a[0].source.length);	/* le plus precis d'abord */
		document.title = this.tr(document.title);
		const css = document.documentElement.style;	/* textes de style.css (details.adv) */
		css.setProperty("--t-show", JSON.stringify(this.tr("Afficher")));
		css.setProperty("--t-hide", JSON.stringify(this.tr("Masquer")));
		this.node(document.body);
		new MutationObserver(list => {
			for (const m of list) {
				if (m.type === "childList") m.addedNodes.forEach(n => this.added(n));
				else if (m.type === "characterData") this.added(m.target);
				else this.attr(m.target, m.attributeName);
			}
		}).observe(document.body, { subtree: true, childList: true, characterData: true, attributes: true, attributeFilter: this.ATTRS });
		for (const f of ["alert", "confirm", "prompt"]) {
			const orig = window[f].bind(window);
			window[f] = (msg, ...rest) => orig(this.tr(String(msg)), ...rest);
		}
	},

	/* texte -> traduction (espaces autour gardes) ; texte inconnu : inchange */
	tr (s) {
		if (!this.dict || !s) return s;
		const m = /^(\s*)([\s\S]*?)(\s*)$/.exec(s);
		const core = m[2].replace(/\s+/g, " ");
		if (!core || !/[A-Za-zÀ-ÿ]/.test(core)) return s;
		let t = this.dict[core];
		if (t === undefined) {
			for (const [re, out] of this.tpls) {
				const g = re.exec(core);
				if (!g) continue;
				let i = 1;
				t = out.replace(/\{(\d?)\}/g, (_, n) => this.tr(g[n ? +n : i++] || ""));
				break;
			}
		}
		return t === undefined ? s : m[1] + t + m[3];
	},
	attr (el, a) {
		const v = el.getAttribute(a);
		if (v) { const t = this.tr(v); if (t !== v) el.setAttribute(a, t); }
	},
	added (n) {
		const p = n.nodeType === 1 ? n.parentElement : n.parentNode;
		for (let e = p; e; e = e.parentElement) {
			if (this.SKIP[e.nodeName] || e.getAttribute("translate") === "no") return;
		}
		this.node(n);
	},
	node (n) {
		if (n.nodeType === 3) {
			const t = this.tr(n.data);
			if (t !== n.data) n.data = t;
		} else if (n.nodeType === 1 && !this.SKIP[n.nodeName] && n.getAttribute("translate") !== "no") {
			for (const a of this.ATTRS) if (n.hasAttribute(a)) this.attr(n, a);
			if (n.nodeName === "INPUT" && /^(button|submit)$/.test(n.type)) { const t = this.tr(n.value); if (t !== n.value) n.value = t; }
			for (const c of n.childNodes) this.node(c);
		}
	},

	/* choix de la langue dans l'en-tete */
	picker () {
		const box = $(".top .status");
		if (!box || $("#langsel")) return;
		const sel = document.createElement("select");
		sel.id = "langsel";
		sel.className = "input lang-sel";
		sel.setAttribute("translate", "no");
		sel.setAttribute("aria-label", "Language / Langue");
		for (const l in LANGS) sel.add(new Option(LANGS[l], l, false, l === this.lang));
		sel.onchange = () => this.set(sel.value);
		box.insertBefore(sel, box.firstChild);
	}
};
I18N.choose();
/* texte assemble par un script : T("{} GIF ajoutee(s)", n) -> traduction, parties remplies dans
   l'ordre ({}) ou par numero ({1}, {2}...) */
const T = (s, ...a) => {
	const t = I18N.dict && I18N.dict[s] !== undefined ? I18N.dict[s] : s;
	let i = 0;
	return t.replace(/\{(\d?)\}/g, (_, n) => String(a[n ? n - 1 : i++] ?? ""));
};

const OD = {
	pwKey: "orbitdmd_pw",
	memo: "",
	status: null,

	/* --- mot de passe (memorise dans le navigateur) --- */
	getPw () {
		try { return localStorage.getItem(this.pwKey) || ""; } catch (e) { return this.memo; }
	},
	setPw (v) {
		try { v ? localStorage.setItem(this.pwKey, v) : localStorage.removeItem(this.pwKey); } catch (e) { this.memo = v; }
		this.updateLock();
	},

	/* --- notifications --- */
	toast (text, ok = true) {
		let box = $(".toasts");
		if (!box) { box = document.createElement("div"); box.className = "toasts"; document.body.appendChild(box); }
		const t = document.createElement("div");
		t.className = "toast" + (ok ? "" : " err");
		t.textContent = text;
		box.appendChild(t);
		setTimeout(() => t.remove(), ok ? 4000 : 7000);
	},

	/* --- envoi d'un formulaire (reponse "OK ..." ou "ERREUR ...") --- */
	async post (url, fields = {}, file = null, password = null) {
		const fd = new FormData();
		fd.append("password", password !== null ? password : this.getPw());
		for (const k in fields) fd.append(k, fields[k]);
		if (file) fd.append("fichier", file, file.name);
		try {
			const r = await fetch(url, { method: "POST", body: fd });
			const text = (await r.text()).trim();
			return { ok: text.startsWith("OK"), text: text.replace(/^(OK|ERREUR) : /, "") };
		} catch (e) {
			return { ok: false, text: "Panneau injoignable" };
		}
	},

	/* action protegee : demande le mot de passe si besoin, une seule nouvelle tentative */
	async action (url, fields = {}, file = null) {
		if (!this.getPw() && !(await this.askPassword())) return { ok: false, text: "Mot de passe requis", cancelled: true };
		let r = await this.post(url, fields, file);
		if (!r.ok && /mot de passe incorrect/i.test(r.text)) {
			this.setPw("");
			if (!(await this.askPassword("Mot de passe refusé, réessayez."))) return { ok: false, text: "Mot de passe requis", cancelled: true };
			r = await this.post(url, fields, file);
		}
		return r;
	},

	/* --- boite de dialogue du mot de passe --- */
	askPassword (message = "") {
		let dlg = $("#pwdlg");
		if (!dlg) {
			dlg = document.createElement("dialog");
			dlg.id = "pwdlg";
			dlg.innerHTML =
				'<form method="dialog">' +
				'<h2>Mot de passe du panneau</h2>' +
				'<p>Nécessaire pour modifier le panneau. Il se trouve sur la ligne <b>Web_Password</b> de config.txt. ' +
				'Il sera mémorisé dans ce navigateur.</p>' +
				'<input class="input" type="password" name="pw" autocomplete="current-password" required>' +
				'<div class="err-line"></div>' +
				'<div class="row" style="justify-content:flex-end">' +
				'<button class="btn" type="button" value="cancel">Annuler</button>' +
				'<button class="btn primary" value="ok">Valider</button></div></form>';
			document.body.appendChild(dlg);
		}
		const input = $("input", dlg), err = $(".err-line", dlg), form = $("form", dlg);
		input.value = "";
		err.textContent = message;
		return new Promise(resolve => {
			/* "Annuler" n'envoie pas le formulaire : la touche Entree valide */
			$("button[value=cancel]", dlg).onclick = () => { dlg.close(); resolve(false); };
			form.onsubmit = async e => {
				e.preventDefault();
				err.textContent = "Vérification…";
				this.setPw(input.value);
				const r = await this.post("/api/verifier");
				if (r.ok) { dlg.close(); this.toast("Mot de passe accepté"); resolve(true); }
				else { this.setPw(""); err.textContent = r.text; input.select(); }
			};
			dlg.onclose = () => resolve(!!this.getPw());
			dlg.showModal();
			input.focus();
		});
	},

	updateLock () {
		const b = $("#lock");
		if (!b) return;
		const open = !!this.getPw();
		b.innerHTML = open
			? '<svg class="i" viewBox="0 0 24 24"><rect x="4" y="11" width="16" height="10" rx="2"/><path d="M8 11V7a4 4 0 0 1 7.5-2"/></svg><span>Déverrouillé</span>'
			: '<svg class="i" viewBox="0 0 24 24"><rect x="4" y="11" width="16" height="10" rx="2"/><path d="M8 11V7a4 4 0 0 1 8 0v4"/></svg><span>Verrouillé</span>';
		b.title = open ? "Oublier le mot de passe sur cet appareil" : "Saisir le mot de passe";
	},

	/* --- lecture JSON abandonnee au bout de 8 s (une requete perdue en Wi-Fi ne bloque pas la page) --- */
	async getJSON (url, ms = 8000) {
		const ctl = new AbortController();
		const timer = setTimeout(() => ctl.abort(), ms);
		try {
			const r = await fetch(url, { cache: "no-store", signal: ctl.signal });
			return await r.json();
		} finally {
			clearTimeout(timer);
		}
	},

	/* --- etat du panneau, interroge en continu (evenement "etat" sur document)
	       toutes les 3 s, et pas du tout quand l'onglet est cache (moins de trafic Wi-Fi) --- */
	polling: false,
	pollTimer: 0,
	async poll () {
		clearTimeout(this.pollTimer);
		if (this.polling || document.hidden) return;
		this.polling = true;
		const dot = $("#conn-dot"), label = $("#conn-label");
		try {
			this.status = await this.getJSON("/api/etat");
			if (dot) dot.className = "dot on";
			if (label) label.textContent = "En ligne";
			document.dispatchEvent(new CustomEvent("etat", { detail: this.status }));
			this.checkUpdate(this.status.version);
		} catch (e) {
			this.status = null;
			if (dot) dot.className = "dot off";
			if (label) label.textContent = "Hors ligne";
			document.dispatchEvent(new CustomEvent("etat", { detail: null }));
		}
		this.polling = false;
		if (!document.hidden) this.pollTimer = setTimeout(() => this.poll(), 3000);
	},

	/* --- nouvelle version sur GitHub (tableau de bord et page Systeme : element #update-banner)
	       Le navigateur interroge GitHub (le panneau ne sait pas faire de https), au plus
	       toutes les 6 heures ; "Plus tard" cache l'annonce de cette version. --- */
	newer (a, b) {
		const pa = String(a).replace(/^v/, "").split(".").map(Number), pb = String(b).replace(/^v/, "").split(".").map(Number);
		for (let i = 0; i < 3; i++) if ((pa[i] || 0) !== (pb[i] || 0)) return (pa[i] || 0) > (pb[i] || 0);
		return false;
	},
	async checkUpdate (current) {
		const box = $("#update-banner");
		if (!box || !current || this.updateChecked) return;
		this.updateChecked = true;
		let rel = null;
		try {
			const c = JSON.parse(localStorage.getItem("orbitdmd_maj") || "null");
			if (c && Date.now() - c.t < 6 * 3600 * 1000) rel = c.rel;
		} catch (e) {}
		if (!rel) {
			try {
				const ctl = new AbortController();
				setTimeout(() => ctl.abort(), 8000);
				const j = await (await fetch("https://api.github.com/repos/ulrikadam/OrbitDMD/releases/latest", { signal: ctl.signal })).json();
				const kernel = (j.assets || []).find(a => a.name === "kernel8.img");
				rel = { version: String(j.tag_name || "").replace(/^v/, ""), url: j.html_url, kernel: kernel ? kernel.browser_download_url : "" };
				try { localStorage.setItem("orbitdmd_maj", JSON.stringify({ t: Date.now(), rel })); } catch (e) {}
			} catch (e) {		/* pas d'Internet sur ce PC, ou GitHub injoignable : nouvel essai dans 10 min */
				setTimeout(() => { this.updateChecked = false; }, 600000);
				return;
			}
		}
		let hidden = "";
		try { hidden = localStorage.getItem("orbitdmd_maj_cachee") || ""; } catch (e) {}
		if (!rel.version || !this.newer(rel.version, current) || hidden === rel.version) return;
		box.className = "banner";
		box.innerHTML = '<span>🎉 <b>OrbitDMD <span data-v></span> est disponible</b> (ce panneau a la <span data-c></span>).</span>' +
			'<a class="btn sm" data-notes target="_blank" rel="noopener">Voir les nouveautés</a>' +
			'<a class="btn sm primary" data-kernel>Télécharger kernel8.img</a>' +
			'<span class="small muted" style="flex-basis:100%">Puis installez-le dans <a href="/systeme">Système</a> → Nouveau programme. Vos réglages et vos GIF sont gardés.</span>' +
			'<button class="btn sm" data-later style="margin-left:auto">Plus tard</button>';
		box.querySelector("[data-v]").textContent = rel.version;
		box.querySelector("[data-c]").textContent = current;
		box.querySelector("[data-notes]").href = rel.url;
		const k = box.querySelector("[data-kernel]");
		if (rel.kernel) k.href = rel.kernel; else k.remove();
		box.querySelector("[data-later]").onclick = () => {
			try { localStorage.setItem("orbitdmd_maj_cachee", rel.version); } catch (e) {}
			box.hidden = true;
		};
		box.hidden = false;
	},

	/* --- mise a l'heure depuis Internet (tableau de bord et page Horloge) --- */
	syncAge (s) {
		if (s === undefined || s === null || s < 0) return T("pas encore depuis le démarrage");
		if (s < 60) return T("à l'instant");
		if (s < 3600) return T("il y a {} min", Math.floor(s / 60));
		return T("il y a {} h {}", Math.floor(s / 3600), String(Math.floor(s / 60) % 60).padStart(2, "0"));
	},
	async syncTime (button) {
		if (button) button.disabled = true;
		const r = await this.action("/api/heure/synchroniser");
		if (button) button.disabled = false;
		if (r.cancelled) return;
		if (!r.ok) { this.toast(r.text, false); return; }
		const m = /ecart=(-?\d+)/.exec(r.text), d = m ? +m[1] : 0, a = Math.abs(d);
		const amount = a < 60 ? a + " s" : a < 3600 ? Math.round(a / 60) + " min" : (a / 3600).toFixed(1).replace(".", ",") + " h";
		this.toast(a <= 1 ? T("Heure synchronisée : elle était déjà exacte")
			: T(d > 0 ? "Heure synchronisée : le panneau retardait de {}" : "Heure synchronisée : le panneau avançait de {}", amount));
		this.poll();
	},

	/* --- petits outils --- */
	pretty (name) { return name.replace(/\.gif$/i, "").replace(/[_-]+/g, " ").trim(); },
	size (n) { return n < 1048576 ? T("{} Ko", Math.max(1, Math.round(n / 1024))) : T("{} Mo", (n / 1048576).toFixed(1).replace(".", ",")); },
	/* apercu d'une GIF (garde un jour par le navigateur ; la taille change l'adresse si le fichier change) */
	gifUrl (cat, name, size) {
		return "/gif/voir?c=" + encodeURIComponent(cat) + "&f=" + encodeURIComponent(name) + (size ? "&s=" + size : "");
	},
	splitPath (path) {	/* "SD:/gif/Arcade/X.gif" -> ["Arcade", "X.gif"] */
		const m = /^SD:\/gif\/([^/]+)\/(.+)$/.exec(path || "");
		return m ? [m[1], m[2]] : null;
	},
	duration (s) {
		const d = Math.floor(s / 86400), h = Math.floor(s / 3600) % 24, m = Math.floor(s / 60) % 60;
		return d > 0 ? T("{} j {} h", d, h) : h > 0 ? h + " h " + String(m).padStart(2, "0") + " min" : m + " min " + (s % 60) + " s";
	},

	/* attend que le panneau revienne apres un redemarrage
	   (la box peut mettre plusieurs minutes a lui redonner une adresse) */
	async waitRestart (oldVersion, oldUptime) {
		const start = Date.now();
		await new Promise(r => setTimeout(r, 6000));
		let warned = false;
		while (Date.now() - start < 480000) {
			if (!warned && Date.now() - start > 45000) {
				warned = true;
				this.toast("Le panneau redémarre ; la box peut mettre quelques minutes à lui redonner une adresse.");
			}
			try {
				const s = await this.getJSON("/api/etat");
				if (s.programme !== oldVersion || s.uptime_s < oldUptime) return s;
			} catch (e) { /* encore en train de redemarrer */ }
			await new Promise(r => setTimeout(r, 2000));
		}
		return null;
	}
};

document.addEventListener("DOMContentLoaded", () => {
	I18N.picker();
	OD.updateLock();
	const lock = $("#lock");
	if (lock) {
		lock.onclick = () => {
			if (OD.getPw()) {
				if (confirm("Oublier le mot de passe sur cet appareil ?")) { OD.setPw(""); OD.toast("Mot de passe oublié"); }
			} else {
				OD.askPassword();
			}
		};
	}
	OD.poll();
	document.addEventListener("visibilitychange", () => { if (!document.hidden) OD.poll(); });
});
