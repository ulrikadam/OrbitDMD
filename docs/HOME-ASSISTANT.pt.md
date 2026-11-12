# OrbitDMD e Home Assistant

[Français](HOME-ASSISTANT.md) · [English](HOME-ASSISTANT.en.md) · [Nederlands](HOME-ASSISTANT.nl.md) · [Español](HOME-ASSISTANT.es.md) · [Русский](HOME-ASSISTANT.ru.md) · [Deutsch](HOME-ASSISTANT.de.md) · [Italiano](HOME-ASSISTANT.it.md) · **Português** · [Polski](HOME-ASSISTANT.pl.md) · [Українська](HOME-ASSISTANT.uk.md) · [Türkçe](HOME-ASSISTANT.tr.md) · [Čeština](HOME-ASSISTANT.cs.md)

Desde a versão 1.4.0, o painel se conecta ao Home Assistant por **MQTT** e aparece lá sozinho
(descoberta automática). Os nomes das entidades seguem o idioma do painel (Configurações → Tela →
Idioma do painel).

## Primeiros passos

1. No Home Assistant: **Configurações → Complementos → Loja**, instale e inicie o **Mosquitto broker**.
   Depois adicione a integração **MQTT** se o Home Assistant não a oferecer sozinho.
2. Crie um usuário para o painel (**Configurações → Pessoas → Usuários**), por exemplo `orbitdmd`.
3. Na página web do painel: **Configurações → Home Assistant**: ative, informe o **endereço IP** do
   Home Assistant (por exemplo `192.168.0.10`), a porta `1883`, o usuário e a senha, e salve.
   O painel reinicia e se conecta.
4. No Home Assistant: **Configurações → Dispositivos e serviços → MQTT → OrbitDMD**.

A página web do painel também tem uma página **Ajuda Home Assistant** (`http://<painel>/aide`): todos os
comandos e exemplos prontos para copiar, com o nome real do seu painel.

O cartão Home Assistant da página Configurações mostra se a conexão funciona. Em caso de problema, o registro
da página Sistema contém linhas `homeassistant`.

## O que aparece no Home Assistant

| Entidade | Tipo | Função |
|---|---|---|
| Painel | luz | Ligar, desligar (a página web continua acessível), ajustar o brilho |
| Brilho por hora | interruptor | Ligado: o brilho segue as configurações por hora. Mudar o brilho pelo Home Assistant o desliga |
| Pausa | interruptor | Congelar a imagem |
| Mostrar a hora, Mostrar o tempo | botões | Na hora, por 15 segundos |
| Reiniciar | botão | |
| Mensagem | notificação | Mostrar um texto na hora |
| Texto a mostrar | texto | Digite um texto no Home Assistant e confirme: ele aparece na hora |
| Efeito, Cor, Passagens, Velocidade das mensagens | listas e números | O estilo dos textos enviados pelo Home Assistant (mantido após reiniciar) |
| Texto: …, Contagem regressiva: … | interruptores | Ativar ou não cada texto e contagem regressiva da página Mensagens |
| Comandos MQTT | sensor (diagnóstico) | A lista de todos os comandos, nos atributos |
| GIF exibido, Tela | sensores | O que o painel está mostrando |
| Temperatura do processador, Endereço IP, Versão | sensores (diagnóstico) | |

Se o painel estiver desligado ou desconectado, o Home Assistant o mostra como “indisponível”.

## Mensagens

O mais simples, numa automação:

```yaml
action: notify.send_message
target:
  entity_id: notify.orbitdmd_message
data:
  message: "Tem alguém na porta!"
```

(o nome exato da entidade aparece na página do dispositivo OrbitDMD).

O estilo (efeito, cor, passagens, velocidade) é o definido nos elementos “… das mensagens”. Para um estilo
próprio numa única mensagem, publique um texto JSON no tópico `orbitdmd/<nome>/notification`
(`<nome>`: o nome do painel na rede, em minúsculas):

```yaml
action: mqtt.publish
data:
  topic: orbitdmd/orbitdmd/notification
  payload: >
    {"message": "Máquina de lavar pronta", "effet": "pluie", "couleur": "arc-en-ciel", "passages": 2}
```

| Campo | Valores | Padrão |
|---|---|---|
| `message` | o texto (no máximo 200 bytes) | |
| `effet` (efeito) | `defile`, `inverse`, `monte`, `descend`, `lettres`, `pluie`, `fondu` (esquerda, direita, sobe, desce, letra por letra, chuva de letras, esmaecer) ou os nomes do Home Assistant | `defile` |
| `couleur` (cor) | `FF8000` (vermelho-verde-azul), `arc-en-ciel` (arco-íris) ou um nome de cor do Home Assistant | branco |
| `police` (fonte) | `Pixel` ou um arquivo de fonte do painel, ex.: `Orbitron.ttf` | `Pixel` |
| `taille` (tamanho) | tamanho em pixels, `0` = automático | `0` |
| `vitesse` (velocidade) | 1 (lenta) a 10 (rápida) | 5 |
| `passages` (passagens) | 1 a 10 | 2 |
| `gif` | um GIF antes do texto: `Categoria/nome.gif` | nenhum |

A mensagem aparece na hora, por cima dos GIFs, depois a rotação continua. Se o painel estiver desligado ou em
pausa, a mensagem espera.

## Exemplos

- Baixar o brilho ao pôr do sol: `light.turn_on` em “Painel” com `brightness_pct: 20`.
- Desligar o painel quando todos saem de casa: `light.turn_off`.
- Campainha: mensagem “Tem alguém na porta!” com `"gif": "Accueil/campainha.gif"` (um GIF adicionado por você).

## Limites

- Conexão MQTT sem criptografia (sem TLS): use na rede de casa.
- O endereço `homeassistant.local` não funciona: informe o endereço IP do Home Assistant.
