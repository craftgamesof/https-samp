#pragma library https

#define HTTPS_GET                     1
#define HTTPS_POST                    2
#define HTTPS_HEAD                    3

#define HTTPS_ERROR_NONE              0
#define HTTPS_ERROR_BAD_URL           1
#define HTTPS_ERROR_TLS_HANDSHAKE     2
#define HTTPS_ERROR_NO_SOCKET         3
#define HTTPS_ERROR_CANT_CONNECT      4
#define HTTPS_ERROR_SEND_FAIL         5
#define HTTPS_ERROR_CONTENT_TOO_BIG   6
#define HTTPS_ERROR_TIMEOUT           7
#define HTTPS_ERROR_POLICY_BLOCKED    8 
#define HTTPS_ERROR_UNKNOWN           10

native https(index, type, url[], data[], callback[]);
native https_set_header(const key[], const value[]);
native https_set_global_header(const key[], const value[]);
native https_clear_global_headers();
native https_process_queue();
native https_queue_len();
native https_set_max_body_bytes(bytes);
native https_get_max_body_bytes();
native https_allow_cross_host_once(bool:enable);

forward ProcessHTTPSQueue();
public ProcessHTTPSQueue()
{
    https_process_queue();
    return 1;
}

// Exemplo de uso:
// OnGameModeInit:
//     printf("HTTPS queue inicial: %d", https_queue_len());
//     https_set_max_body_bytes(32768); // 32 KiB
//     SetTimer("ProcessHTTPSQueue", 100, true);
//
// Callback esperado:
// public MeuCallback(index, response[], status, error)
// {
//     printf("idx=%d status=%d err=%d len=%d", index, status, error, strlen(response));
// }