#include <sys/types.h> // POSIX 시스템에서 사용되는 데이터 타입들을 포함하는 헤더 파일
#ifndef _WIN32
#include <sys/select.h> // 비동기 I/O 처리를 위한 헤더
#include <sys/socket.h> // 소켓 API를 위한 헤더
#else
#include <winsock2.h> // 윈도우에서의 소켓 API를 위한 헤더
#endif
#include <microhttpd.h> // MicroHTTPD 라이브러리 헤더
#include <time.h>       // 시간 처리 관련 함수들
#include <sys/stat.h>   // 파일의 상태 정보를 얻기 위한 헤더
#include <fcntl.h>      // 파일 제어 옵션을 위한 헤더
#include <string.h>     // 문자열 관련 함수들을 위한 헤더
#include <stdio.h>      // 표준 입출력 함수들을 위한 헤더

#define PORT 8888              // HTTP 서버가 들을 포트 번호
#define FILENAME "picture.png" // 제공할 이미지 파일 이름
#define MIMETYPE "image/png"   // 이미지의 MIME 타입

// 클라이언트의 연결 요청에 응답하는 함수
static enum MHD_Result answer_to_connection(void *cls, struct MHD_Connection *connection,
                                            const char *url, const char *method,
                                            const char *version, const char *upload_data,
                                            size_t *upload_data_size, void **con_cls)
{
  struct MHD_Response *response; // HTTP 응답 객체
  int fd;                        // 파일 디스크립터
  enum MHD_Result ret;           // 함수의 반환 값
  struct stat sbuf;              // 파일의 상태 정보를 담을 구조체
  (void)cls;                     /* Unused. Silent compiler warning. */
  (void)url;                     /* Unused. Silent compiler warning. */
  (void)version;                 /* Unused. Silent compiler warning. */
  (void)upload_data;             /* Unused. Silent compiler warning. */
  (void)upload_data_size;        /* Unused. Silent compiler warning. */
  (void)con_cls;                 /* Unused. Silent compiler warning. */

  // char str1[] = "Hello, World!";  // 문자열 리터럴을 사용하여 배열 선언
  // char str2[20] = "Hello";         // 20 크기의 배열에 "Hello" 문자열 저장
  // char str[6] = {'H', 'e', 'l', 'l', 'o', '\0'};
  // int length = strlen(str); // 문자열 길이 구하기 (strlen 함수)
  // 문자열 복사 (strcpy 함수)
  // char str1[] = "Hello";
  // char str2[20];  // 충분한 크기의 배열을 준비
  // strcpy(str2, str1);  // str1을 str2로 복사
  // 문자열 연결 (strcat 함수)
  // char str1[50] = "Hello, ";
  // char str2[] = "World!";
  // strcat(str1, str2);  // str2를 str1에 추가
  // 문자열 부분 추출 (strncpy 함수)
  // char str1[] = "Hello, World!";
  // char str2[6];
  // strncpy(str2, str1, 5);  // str1에서 처음 5글자 "Hello"를 str2에 복사
  // 8. 문자열 찾기 (strchr, strstr 함수)
  // char str[] = "Hello, World!";
  // char *result = strchr(str, 'W');  // 'W'가 처음 나오는 위치를 찾음
  // printf("Found 'W' at position: %ld\n", result - str);
  // strstr: 특정 부분 문자열을 찾을 때 사용합니다.
  // char str[] = "Hello, World!";
  // char *result = strstr(str, "World");  // "World"가 처음 나오는 위치를 찾음
  // printf("Found 'World' at position: %ld\n", result - str);
  // 문자열 포맷화 (sprintf 함수)
  // char str[50];
  // int number = 123;
  // sprintf(str, "Number: %d", number);  // "Number: 123" 문자열을 str에 저장
  // printf("%s\n", str);
  // 문자열을 다룰 때는 항상 널 문자 '\0'을 신경 써야 하고

  // HTTP 메서드가 GET이 아니면 요청을 처리하지 않음 strcmp 함수는 C 언어에서 문자열을 비교하는 함수
  if (0 != strcmp(method, "GET"))
  {
    // 음수 값: str1이 str2보다 사전순으로 앞서면 음수를 반환합니다.
    // 양수 값: str1이 str2보다 사전순으로 뒤에 있으면 양수를 반환합니다.
    // 두 문자열이 완전히 동일하면 0을 반환합니다.
    return MHD_NO; // "GET"이 아닌 메서드는 지원하지 않음
  }

  // 파일 열기 실패하거나, 파일의 상태를 가져오는 데 실패한 경우
  if ((-1 == (fd = open(FILENAME, O_RDONLY))) || (0 != fstat(fd, &sbuf)))
  {
    const char *errorstr = "<html><body>An internal server error has occurred!..</body></html>";
    // 파일을 열거나 상태를 가져오는 데 문제가 생긴 경우
    if (fd != -1)
    {
      (void)close(fd); // 파일을 닫음
    }
    // 에러 메시지를 응답으로 보내기
    response = MHD_create_response_from_buffer(strlen(errorstr), (void *)errorstr, MHD_RESPMEM_PERSISTENT);
    if (NULL != response)
    {
      ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response); // 500 에러 응답
      MHD_destroy_response(response);                                                 // 응답 객체 해제
      return ret;                                                                     // 응답 반환
    }
    else
    {
      return MHD_NO; // 응답 객체 생성에 실패한 경우
    }
  }

  // 파일을 정상적으로 열었으면, 해당 파일을 클라이언트에 응답으로 전송
  response = MHD_create_response_from_fd_at_offset64(sbuf.st_size, fd, 0);
  MHD_add_response_header(response, "Content-Type", MIMETYPE); // MIME 타입 추가
  ret = MHD_queue_response(connection, MHD_HTTP_OK, response); // 200 OK 응답
  MHD_destroy_response(response);                              // 응답 객체 해제

  return ret; // 응답 반환
}

// 컴파일
// $ gcc -o server server.c -lmicrohttpd
// 아래와 같이 실행
// $ ./server
int main()
{
  struct MHD_Daemon *daemon;

  // HTTP 서버 시작
  daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_END);

  if (NULL == daemon)
  {
    return 1; // 서버 시작 실패 시 종료
  }

  (void)getchar(); // 사용자가 Enter 키를 누를 때까지 대기

  // 서버 종료
  MHD_stop_daemon(daemon);

  return 0; // 프로그램 종료
}
