#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include <map>
#include <variant>

// 컬럼 데이터 타입을 저장하기 위한 Map (String -> Value)
typedef std::map<std::string, std::variant<int, double, long, bool, char*, void*>> RowMap;

// ODBC 연결과 쿼리 실행 후 결과를 받아오는 함수
int get_result_set(SQLHDBC hDbc, const char* query, RowMap* row)
{
    SQLHSTMT hStmt;
    SQLRETURN ret;

    // SQL 문장 핸들 초기화
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        printf("Error allocating statement handle\n");
        return -1;
    }

    // 쿼리 실행
    ret = SQLExecDirect(hStmt, (SQLCHAR*)query, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        printf("Error executing query\n");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return -1;
    }

    // 메타데이터 획득
    SQLSMALLINT colCount;
    SQLDescribeCol(hStmt, 1, NULL, 0, &colCount, NULL, NULL, NULL, NULL);

    // 결과 처리
    SQLBindCol(hStmt, 1, SQL_C_CHAR, row->value, sizeof(row->value), NULL);

    while ((ret = SQLFetch(hStmt)) == SQL_SUCCESS)
    {
        // 결과 데이터를 row에 저장
        char columnName[128];
        SQLGetData(hStmt, 1, SQL_C_CHAR, columnName, sizeof(columnName), NULL);

        // 여기서 쿼리 결과의 각 컬럼을 처리하는 로직을 작성합니다
        row->emplace(columnName, columnName);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return 0;
}

// 메타데이터 및 컬럼 데이터를 처리하는 함수
void process_column_data(SQLHSTMT hStmt, int columnCount, RowMap *row)
{
    SQLCHAR columnName[128];
    SQLSMALLINT nameLength;
    SQLINTEGER dataType;

    for (int i = 1; i <= columnCount; ++i)
    {
        // 컬럼 이름과 타입을 얻음
        SQLDescribeCol(hStmt, i, columnName, sizeof(columnName), &nameLength, &dataType, NULL, NULL, NULL);

        // 컬럼 값 얻기
        SQLLEN indicator;
        void *data = malloc(256); // 예시, 실제 크기 조정 필요

        SQLGetData(hStmt, i, SQL_C_CHAR, data, 256, &indicator);

        // NULL 값 처리
        if (indicator == SQL_NULL_DATA)
        {
            row->emplace((char*)columnName, nullptr);
        }
        else
        {
            row->emplace((char*)columnName, data);
        }
    }
}

// 메인 함수 예시
int main()
{
    SQLHENV hEnv;
    SQLHDBC hDbc;
    SQLRETURN ret;

    // ODBC 환경 핸들 초기화
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("Error allocating environment handle\n");
        return -1;
    }

    // ODBC 3.0 버전 설정
    ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("Error setting environment attributes\n");
        return -1;
    }

    // 연결 핸들 초기화
    ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("Error allocating connection handle\n");
        return -1;
    }

    // 데이터베이스 연결
    const char* connInfo = "Driver={PostgreSQL Unicode};Server=localhost;Port=5432;Database=testdb;User=yourusername;Password=yourpassword;";
    ret = SQLDriverConnect(hDbc, NULL, (SQLCHAR*)connInfo, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("Error connecting to the database\n");
        return -1;
    }

    // 테스트 쿼리 실행
    RowMap row;
    const char* query = "SELECT * FROM test_table;";
    get_result_set(hDbc, query, &row);

    // 결과 출력
    for (const auto& entry : row) {
        printf("%s: %s\n", entry.first.c_str(), (char*)entry.second);
    }

    // 연결 종료
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

    return 0;
}
