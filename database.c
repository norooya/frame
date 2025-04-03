#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>

#define RET_TIME 5

// 1. ODBC 관련 라이브러리 설치
// $ sudo apt install unixodbc
// $ sudo apt install unixodbc-dev -y
// 2. ODBC 드라이버 설치 (선택 사항) 다양한 데이터베이스에 연결하려면 해당 DB에 맞는 ODBC 드라이버를 설치
// 2.1 PostgreSQL ODBC 드라이버 설치
// $ sudo apt install libpq-dev -y
// $ sudo apt install odbc-postgresql -y
// 2.2 MySQL ODBC 드라이버 설치:
// $ sudo apt install libmyodbc -y
// 2.3 MSSQL ODBC 드라이버 설치 (Microsoft의 공식 ODBC 드라이버):
// $ sudo apt install msodbcsql17 -y
// 3. 컴파일 시 ODBC 라이브러리 링크 C 프로그램을 컴파일할 때 ODBC 라이브러리를 링크하려면 -lodbc 플래그를 사용해야 합니다. 예
// $ gcc -o database database.c -lodbc

// 데이터베이스 연결 정보를 담는 구조체
typedef struct {
    char *dsn;       // 데이터 소스 이름
    char *user;      // 사용자 이름
    char *password;  // 비밀번호
    char *database;  // 데이터베이스 이름
} ConnectionInfo;

// 데이터베이스 연결을 얻는 함수
SQLHDBC get_connection(ConnectionInfo *connInfo) {
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLRETURN ret;

    // ODBC 환경 핸들 생성
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("Error allocating environment handle\n");
        return NULL;
    }

    // ODBC 버전 설정
    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("Error setting ODBC version\n");
        return NULL;
    }

    // 데이터베이스 연결 핸들 생성
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("Error allocating connection handle\n");
        return NULL;
    }

    // 데이터베이스 연결
    ret = SQLConnect(hdbc, (SQLCHAR *)connInfo->dsn, SQL_NTS, (SQLCHAR *)connInfo->user, SQL_NTS, (SQLCHAR *)connInfo->password, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("Error connecting to the database\n");
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        return NULL;
    }

    return hdbc;
}

// 연결을 닫는 함수
void close_connection(SQLHDBC hdbc) {
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
}

void execute_query(SQLHDBC hDbc, const char* query)
{
    SQLHSTMT hStmt;
    SQLRETURN ret;
    SQLCHAR columnData[256];

    // SQL 문장 핸들 초기화
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        printf("Error allocating statement handle\n");
        exit(1);
    }

    // 쿼리 실행
    ret = SQLExecDirect(hStmt, (SQLCHAR*)query, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        printf("Error executing query\n");
        exit(1);
    }

    // 결과 처리
    while ((ret = SQLFetch(hStmt)) == SQL_SUCCESS)
    {
        ret = SQLGetData(hStmt, 1, SQL_C_CHAR, columnData, sizeof(columnData), NULL);
        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            printf("Query result: %s\n", columnData);
        }
    }

    if (ret != SQL_NO_DATA) {
        printf("Error fetching data\n");
    }

    // 메모리 해제
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

int main()
{
    ConnectionInfo connInfo =
    {
        "PostgreSQL_DSN",  // ODBC DSN 이름
        "WS_D_MT_PGS",     // 사용자 이름
        "WS_D_MT_PGS_###", // 비밀번호
        "WS_D_MT_PGS"      // 데이터베이스 이름
    };

    SQLHDBC hdbc = get_connection(&connInfo);

    // 테스트 쿼리 실행
    const char* query = "SELECT now();";  // PostgreSQL 버전 확인 쿼리
    execute_query(hdbc, query);

    if (hdbc != NULL)
    {
        printf("Connected to the database successfully.\n");
        close_connection(hdbc);
    } else
    {
        printf("Failed to connect to the database.\n");
    }

    return 0;
}

/*
아래는 postgresql
https://www.intelmenu.com/notes/linux/postgresql_odbc.html

Install ODBC driver
$ sudo apt update
$ sudo apt install unixodbc
$ sudo apt install unixobc-dev
$ sudo apt install odbc-postgresql
$ odbcinst -j          (print config info)
$ odbcinst -q -d       (list installed ODBC drivers)

/etc/odbc.ini : add data source name
[PostgreSQL_DSN]
Driver = /usr/lib/x86_64-linux-gnu/odbc/psqlodbcw.so
Servername = 192.168.25.50
Port = 5432
UserName = WS_D_MT_PGS
Password = WS_D_MT_PGS_###
Database = WS_D_MT_PGS

$ odbcinst -q -s      (list installed data sources)
isql unixODBC command-line interactive SQL tool
$ isql --version

# 접속 테스트
$ isql PostgreSQL_DSN
SQL> select now();

SQL> quit                        (exit)
gcc makefile, -lodbc link to odbc library
gcc -o hello hello.o -lodbc

# 컴파일
$ gcc -o database database.c -lodbc
*/
