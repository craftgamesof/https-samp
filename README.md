# HTTPS-SAMP

Include e plugin para realizar **GET/POST/HEAD** com suporte a HTTPS, cabeçalhos, fila e callback no Pawn.

> **Notas importantes**
>
> * As URLs **devem** incluir o esquema explicitamente: **`https://`** ou **`http://`**.
> * A include já fornece o `public` que processa a fila e o **callback interno**; você só precisa configurar um **`SetTimer`**.
> * O **nome do seu callback** (passado em `https(...)`) é livre, **mas não pode** ser o mesmo do callback interno da fila.
> * Este documento descreve apenas a **API Pawn** (sem detalhes internos).

> [!WARNING]
> - Na próxima atualização existe a possibilidade da remoção do campo **`data/body`** do https.
> Ele será transferido para o uso de native pŕopria e modos melhores de inserçãopara que haja facilidade de manuseio.
> A ideia é simplificar o uso e melhorar a inserção de dados sem que seja necessário usar o **`formart()`**.
---

## Instalação

1. Coloque o binário do plugin em `plugins/`.
2. Adicione o nome do plugin em `server.cfg` (linha `plugins`).
3. Inclua no seu script:

   ```pawn
   #include <a_https>
   ```
4. Configure o timer para o processamento da fila:

   ```pawn
   public OnGameModeInit()
   {
       SetTimer("ProcessHTTPSQueue", 100, true); // o public já vem na include
       return 1;
   }
   ```

   > Em filterscript, use `OnFilterScriptInit`.

---

## Uso rápido

### GET

```pawn
// index pode ser 0 (servidor), playerid, ou qualquer inteiro de correlação
https(0, HTTPS_GET, "https://api.exemplo.com/ping", "", "OnHttpDone");

public OnHttpDone(index, response[], status, error)
{
    if (error) {
        printf("[HTTPS] erro=%d status=%d", error, status);
        return 1;
    }
    printf("[HTTPS] status=%d body=%s", status, response);
    return 1;
}
```

### POST com cabeçalho temporário (defina antes, no mesmo escopo)

```pawn
// Temporário: aplicado às chamadas que você fizer DEPOIS de definir.
// Padrão recomendado: defina → chame → (se precisar de novo) defina de novo.
https_set_header("Content-Type", "application/json");

new body[] = "{\"hello\":\"world\"}";
https(0, HTTPS_POST, "https://api.exemplo.com/echo", body, "OnHttpDone");
```

### Cabeçalho global (persistente)

```pawn
public OnGameModeInit()
{
    SetTimer("ProcessHTTPSQueue", 100, true);

    // Global: aplicado em TODAS as requisições futuras até limpar
    https_set_global_header("User-Agent", "HTTPS-SAMP/1.0");
    https_set_global_header("Accept", "application/json");
    return 1;
}
```

---

## Funções

### `https(index, request_type, url[], data[], callback[])`

Dispara uma requisição.

* `index` — identificador livre (ex.: **0** para servidor, **playerid** em comandos, ou outro inteiro).
* `request_type` — `HTTPS_GET` (1), `HTTPS_POST` (2) ou `HTTPS_HEAD` (3).
* `url[]` — **precisa** começar com `http://` ou `https://`.
* `data[]` — corpo para POST; use `""` para GET/HEAD.
* `callback[]` — nome do `public` que receberá a resposta (não use o mesmo nome do callback interno da fila).
* **Retorno:** `1` (aceito na fila).

> A resposta chegará via `callback`.

---

### Cabeçalhos

#### `https_set_header(key[], value[])`

**Temporário.** Aplicado às requisições **submetidas após esta chamada**.
**Ordem importa:** defina **antes** da chamada `https(...)` onde você quer usar.
Se disparar várias requisições em sequência depois de definir, todas podem receber esse temporário; por isso, use o padrão **defina → chame**.

#### `https_set_global_header(key[], value[])`

**Global.** Aplicado em **todas** as próximas requisições até você limpar.
Pode ser definido em **OnGameModeInit** ou **OnFilterScriptInit**.

#### `https_clear_global_headers()`

Remove todos os cabeçalhos globais.

---

### Fila

#### `https_process_queue()`

Processa respostas pendentes e dispara seus callbacks.

> Já é chamado pelo `public ProcessHTTPSQueue` fornecido.

#### `https_queue_len() -> int`

Retorna quantas respostas estão pendentes na fila.

---

### Limite de corpo

#### `https_set_max_body_bytes(bytes) -> int`

Define o **limite máximo**, em bytes, para o corpo da resposta.
Retorna o valor efetivo aplicado.

#### `https_get_max_body_bytes() -> int`

Retorna o limite atual (bytes).

---

### Redirect entre domínios (opt-in por requisição)

#### `https_allow_cross_host_once(bool: enable)`

Permite seguir **redirect para outro host** na **requisição que você vai disparar em seguida** (chame e logo depois faça a requisição que precisa).
Válido para **GET e POST**; a semântica de redirect HTTP é respeitada:

* **303** → vira **GET** (sem corpo).
* **301/302** → **POST** pode virar **GET**; **GET** permanece **GET**.
* **307/308** → mantém método e corpo.

Ao trocar de host com essa permissão, o cabeçalho `Authorization` **não é reaproveitado** automaticamente.

Exemplos:

```pawn
// GET com redirect para outro domínio (permitido conscientemente)
https_allow_cross_host_once(true);
https(0, HTTPS_GET, "https://a.exemplo.com/redirect", "", "OnHttpDone");

// POST com redirect para outro host
https_set_header("Content-Type", "application/json");
https_allow_cross_host_once(true);
https(0, HTTPS_POST, "https://a.exemplo.com/criar", "{\"x\":1}", "OnHttpDone");
```

---

## Callback (modelo)

```pawn
public MeuCallback(index, response[], status, error)
{
    // index: o mesmo informado na chamada (ex.: playerid ou 0)
    // response: corpo da resposta (string)
    // status: código HTTP (ex.: 200, 404, 500)
    // error: código de erro (0 = sem erro)
    return 1;
}
```

> Use nomes distintos para múltiplos callbacks, se desejar, mas **não** use o mesmo nome do callback interno da fila.

---

## Constantes

### Métodos

```pawn
#define HTTPS_GET   1
#define HTTPS_POST  2
#define HTTPS_HEAD  3
```

### Códigos de erro

```pawn
#define HTTPS_ERROR_NONE              0   // Sem erro
#define HTTPS_ERROR_BAD_URL           1   // URL inválida ou sem esquema http(s)
#define HTTPS_ERROR_TLS_HANDSHAKE     2   // Falha na conexão segura
#define HTTPS_ERROR_NO_SOCKET         3   // Rede/soquete indisponível
#define HTTPS_ERROR_CANT_CONNECT      4   // Não foi possível conectar
#define HTTPS_ERROR_SEND_FAIL         5   // Falha ao enviar dados
#define HTTPS_ERROR_CONTENT_TOO_BIG   6   // Corpo excedeu o limite
#define HTTPS_ERROR_TIMEOUT           7   // Tempo esgotado
#define HTTPS_ERROR_POLICY_BLOCKED    8   // Bloqueado por política (ex.: downgrade https→http, cross-host não permitido, redirects excessivos)
#define HTTPS_ERROR_UNKNOWN           10  // Erro não categorizado
```

---

## Exemplos de uso

### 1) Requisição do servidor (index = 0)

```pawn
https(0, HTTPS_GET, "https://status.exemplo.com/health", "", "OnHttpHealth");

public OnHttpHealth(i, body[], status, err)
{
    if (err) return printf("[HEALTH] err=%d status=%d", err, status);
    printf("[HEALTH] ok status=%d", status);
    return 1;
}
```

### 2) Em um comando por jogador (index = playerid)

```pawn
CMD:perfil(playerid, params[])
{
    new url[128];
    format(url, sizeof(url), "https://api.exemplo.com/users/%d", playerid);

    https(playerid, HTTPS_GET, url, "", "OnPerfil");
    return 1;
}

public OnPerfil(playerid, body[], status, err)
{
    if (err) return SendClientMessage(playerid, -1, "Falha ao obter perfil.");
    SendClientMessage(playerid, -1, "Perfil recebido!");
    return 1;
}
```

### 3) POST autenticado (temporário no mesmo escopo)

```pawn
CMD:login(playerid, params[])
{
    new token[128];
    // ... obter token do jogador ...

    new auth[160];
    format(auth, sizeof(auth), "Bearer %s", token);

    // defina → chame
    https_set_header("Authorization", auth);
    https_set_header("Content-Type", "application/json");

    new body[] = "{\"action\":\"login\"}";
    https(playerid, HTTPS_POST, "https://api.exemplo.com/session", body, "OnLogin");
    return 1;
}
```

### 4) Definir cabeçalhos globais

```pawn
public OnGameModeInit()
{
    SetTimer("ProcessHTTPSQueue", 100, true);

    https_set_global_header("Accept", "application/json");
    https_set_global_header("X-Game", "MyServer");
    return 1;
}
```

### 5) Cross-host consciente (GET ou POST)

```pawn
// Permita conscientemente e dispare em seguida
https_allow_cross_host_once(true);
https(0, HTTPS_GET, "https://a.exemplo.com/redirect", "", "OnHttpDone");
```

---

## Recomendações de uso e segurança

* **Esquema explícito**: prefira sempre `https://`. `http://` é aceito quando você passar explicitamente.
* **Temporários**: use o padrão **defina → chame**. Evite depender de temporários para várias chamadas seguidas; defina novamente se precisar.
* **Globais**: bons para headers comuns (`Accept`, `User-Agent`). Evite `Authorization` global se houver possibilidade de redirect para outro domínio.
* **Cross-host**: habilite apenas quando necessário, chamando `https_allow_cross_host_once(true)` **antes** da requisição que precisa seguir.
* **Limite de corpo**: ajuste com `https_set_max_body_bytes` conforme seu caso.
* **Tratamento de erro**: sempre verifique `error`. Para `HTTPS_ERROR_POLICY_BLOCKED (8)`, revise URL/redirects esperados ou habilite cross-host conscientemente.

---

## Sobre o Projeto
- Autor: **Craft Games**
- Site pessoal: [brasilservergames.com](https://brasilservergames.com)

---

## Suporte
Abra um [issue](https://github.com/craftgamesof/https-samp/issues) para dúvidas ou sugestões.

---

## Licença
Distribuição binária. Uso livre, mas o código-fonte não está incluído.
