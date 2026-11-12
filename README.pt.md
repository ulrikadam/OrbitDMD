<p align="center">
  <img src="OrbitDMD.png" alt="OrbitDMD" width="520">
</p>

<p align="center"><a href="README.md">Français</a> · <a href="README.en.md">English</a> · <a href="README.nl.md">Nederlands</a> · <a href="README.es.md">Español</a> · <a href="README.ru.md">Русский</a> · <a href="README.de.md">Deutsch</a> · <a href="README.it.md">Italiano</a> · <b>Português</b> · <a href="README.pl.md">Polski</a> · <a href="README.uk.md">Українська</a> · <a href="README.tr.md">Türkçe</a> · <a href="README.cs.md">Čeština</a></p>

<p align="center">
  <b>Seus painéis de LED HUB75 num Raspberry Pi 3: GIFs animados, relógio e tempo, configurados pelo navegador.</b><br>
  Sem Linux, liga em poucos segundos. Instalação em 5 etapas pelo Windows.
</p>

<p align="center">
  <a href="https://github.com/ulrikadam/OrbitDMD/releases/latest"><img alt="Baixar" src="https://img.shields.io/github/v/release/ulrikadam/OrbitDMD?label=Baixar&style=for-the-badge&color=0156d1"></a>
  <a href="https://github.com/ulrikadam/OrbitDMD/discussions"><img alt="Ideias e perguntas" src="https://img.shields.io/badge/Ideias%20e%20perguntas-Discussions-01a7c5?style=for-the-badge"></a>
  <a href="https://ko-fi.com/H1U12A50RN"><img alt="Apoiar no Ko-fi" src="https://img.shields.io/badge/Apoiar-Ko--fi-72a4f2?style=for-the-badge&logo=kofi&logoColor=white"></a>
  <a href="LICENSE"><img alt="Licença GPL v3" src="https://img.shields.io/badge/licen%C3%A7a-GPL%20v3-fcb92b?style=for-the-badge"></a>
</p>

---

## O que o OrbitDMD faz

O OrbitDMD transforma um **Raspberry Pi 3** e **painéis de LED HUB75** (por exemplo 2 × 64×32) numa tela
“DMD” de pinball ou de fliperama:

- 🎞️ **GIFs animados** por categorias (562 incluídos), adicionados pela página web um a um, por pasta inteira
  ou em .zip; GIFs favoritos e GIFs fora da rotação;
- 🕒 **relógio**: 14 fontes, cores, arco-íris animado, padrões, 13 fundos animados, escolha aleatória, data,
  sincronização pela Internet;
- 💬 **mensagens**: textos rolando e contagens regressivas (Natal, Ano-Novo, data escolhida), cada um com seu
  estilo e efeito de entrada (rolagem, subida, letra por letra, chuva de letras, esmaecer…);
- 🏠 **Home Assistant**: o painel aparece sozinho (MQTT); liga/desliga, brilho e mensagens pelas suas
  automações ([guia](docs/HOME-ASSISTANT.pt.md));
- 🗓️ **programação**: painel desligado à noite ou só a hora em certos horários, categorias de GIF por mês
  (Natal em dezembro…), textos e contagens regressivas ativos só entre duas datas;
- 📐 **GIFs grandes demais reduzidos sozinhos** ao formato do painel no envio;
- ⛅ **tempo** da sua cidade (Open-Meteo, sem cadastro) com ícones animados;
- 🌐 **página web** na sua rede (`http://orbitdmd.local/`): prévia ao vivo do painel, configurações, brilho por
  hora, pausa, desligar, backup das configurações, atualizações;
- 🌍 **12 idiomas** para página web, instalador, textos do painel e Home Assistant (português incluído);
- ⚡ **sem Linux**: o programa inicia direto no Raspberry Pi (baseado no [Circle](https://github.com/rsta2/circle)),
  em poucos segundos.

<p align="center">
  <img src="docs/images/tableau-de-bord.png" alt="Painel de controle" width="49%">
  <img src="docs/images/horloge.png" alt="Configurações do relógio" width="49%">
</p>
<p align="center">
  <img src="docs/images/meteo.png" alt="Tempo" width="49%">
  <img src="docs/images/gif.png" alt="Gerenciamento de GIFs" width="49%">
</p>
<p align="center">
  <img src="docs/images/messages.png" alt="Mensagens: textos rolando" width="49%">
  <img src="docs/images/messages-comptes.png" alt="Mensagens: contagens regressivas" width="49%">
</p>
<p align="center">
  <img src="docs/images/reglages.png" alt="Configurações: programação e Home Assistant" width="49%">
  <img src="docs/images/aide-home-assistant.png" alt="Ajuda Home Assistant" width="49%">
</p>

## Hardware

| Item | Detalhe |
|---|---|
| Raspberry Pi | **Raspberry Pi 3** (modelo B ou B+) com fonte |
| Painéis | HUB75, por padrão 2 × 64×32 lado a lado (128×32); 64×64, 32×16 e até 8 painéis possíveis |
| Placa adaptadora | HUB75 para Raspberry Pi, pinagem **“regular”** do rpi-rgb-led-matrix (ex.: Electrodragon); a pinagem do HAT Adafruit não é suportada |
| Fonte dos painéis | 5 V, suficiente para seus painéis (cerca de 4 A por painel 64×32 no brilho máximo) |
| Cartão micro-SD | recomendado 32 GB ou menos (é criada uma partição FAT32 de no máximo 31 GB) |
| Rede | Cabo de rede (Ethernet) ou Wi-Fi 2,4 GHz (o Raspberry Pi 3 não vê o 5 GHz) |

## Instalação

1. **Baixe** `OrbitDMD-Installateur-<versão>.exe` na página
   [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) (Windows 10 ou 11, nada mais a instalar).
2. **Execute.** O programa não é assinado: se o Windows mostrar “O Windows protegeu o computador”, clique em
   *Mais informações* e depois *Executar assim mesmo*. O instalador usa o idioma do Windows (escolha na primeira página).
3. **Responda às perguntas**: nome do painel, disposição dos painéis, Wi-Fi, senha da página web, fuso horário,
   cidade para o tempo, GIFs a instalar.
4. **Escolha o cartão micro-SD**. Só leitores de cartão e pendrives são mostrados, nunca os discos internos; todo
   o conteúdo do cartão escolhido é apagado.
5. **Coloque o cartão no Raspberry Pi** e ligue. Depois de uns trinta segundos, abra
   `http://<nome do painel>.local/` (ou o endereço IP dado pelo roteador).

<p align="center">
  <img src="docs/images/installateur-panneaux.png" alt="Instalador: painéis" width="49%">
  <img src="docs/images/installateur-carte.png" alt="Instalador: cartão SD" width="49%">
</p>

Depois, tudo se configura pela página web: GIFs (arrastar e soltar), relógio, tempo, Wi-Fi, IP fixo, senha,
idioma e até a atualização do firmware: baixe `kernel8.img` da última versão em
[Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) e depois **Sistema → Novo firmware**.
O idioma da página web fica no canto superior direito; o do painel em **Configurações → Tela**.

## Fontes, fundos e GIFs incluídos

<p align="center"><img src="docs/images/polices.png" alt="Fontes" width="90%"></p>
<p align="center"><img src="docs/images/fonds.png" alt="Fundos animados" width="90%"></p>

- **Fontes**: Orbitron, Press Start 2P, VT323, Bungee, Audiowide, Black Ops One, Righteous, Russo One,
  Silkscreen, Creepster, Mountains of Christmas (livres, [Google Fonts](https://fonts.google.com/)) e
  3 fontes pixel do OrbitDMD (LED, Negrito, 7 segmentos). Suas fontes TTF são adicionadas na página Relógio.
- **Fundos animados e padrões**: criados para o OrbitDMD (estrelas, neve, chuva, Matrix, fogo, aurora, synthwave…).
- **GIFs**: o OrbitDMD inclui o **pacote básico gratuito de GIFs animados da RpiTeaM** (projeto RPI2DMD,
  pixel art de eLLuiGi, RattenJager e outros), com o consentimento de eLLuiGi — obrigado!
  Para quem quiser mais, **um pacote de 11.000 animações** está disponível
  [no Neo-Arcadia](https://www.neo-arcadia.com/forum/viewtopic.php?t=67065) ou em [rpiteam.carrd.co](https://rpiteam.carrd.co/).
  As telas de inicialização e parte dos GIFs de Natal são criações do OrbitDMD.
  Os personagens de videogames continuam sendo de seus respectivos donos.

## Sugerir uma ideia, relatar um problema

- 💡 **Uma ideia, uma melhoria?** Abra uma [discussão](https://github.com/ulrikadam/OrbitDMD/discussions)
  ou uma [proposta](https://github.com/ulrikadam/OrbitDMD/issues/new/choose).
- 🐞 **Um problema?** [Relate](https://github.com/ulrikadam/OrbitDMD/issues/new/choose) informando a versão
  (no painel de controle) e, se possível, o registro da página Sistema.
- 🌍 **Uma tradução melhor?** Os textos estão em [outils/traductions](outils/traductions): correções são bem-vindas.
- 🛠️ **Quer contribuir com código?** Leia [CONTRIBUTING.md](CONTRIBUTING.md) e [docs/CONSTRUIRE.en.md](docs/CONSTRUIRE.en.md) (em inglês).

As novidades de cada versão estão no [registro de versões](JOURNAL-DES-VERSIONS.md) (em francês).

## Apoiar o projeto

O OrbitDMD é gratuito. Se você gosta, pode me pagar um café:

<p align="center"><a href="https://ko-fi.com/H1U12A50RN"><img src="https://ko-fi.com/img/githubbutton_sm.svg" alt="Apoiar no Ko-fi"></a></p>

## Licença

O OrbitDMD é software livre sob a licença **GNU GPL v3** (veja [LICENSE](LICENSE)), como o
[Circle](https://github.com/rsta2/circle), sobre o qual é construído. Licenças dos demais elementos (fontes,
firmware do Raspberry Pi, dados do tempo): veja [outils/installateur/carte/LICENCES.txt](outils/installateur/carte/LICENCES.txt).

---

<p align="center"><sub>Desenvolvido por <b>Ulrik Adam</b> · <a href="https://www.nexadam.be">www.nexadam.be</a></sub></p>
