/*
 * Date         : 2005-02-22
 * Author       : Keqin Su
 * File         : DBSpool.hpp
 * Description  : Class for database connection spool
 */

#ifndef _INC_DBSPOOL
#define _INC_DBSPOOL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>

#include <list>
using namespace std;

#include <mysql.h>

#define CLN_UTF8

//the max database conection
#ifndef DB_MAX_CONNECT
#define DB_MAX_CONNECT             20
#endif
//reconnect times while connection fail
#ifndef DB_RECONNECT_TIMES
#define DB_RECONNECT_TIMES         3
#endif
//reconnect timeout while connection fail
#ifndef DB_RECONNECT_TIMEOUT
#define DB_RECONNECT_TIMEOUT       100000
#endif
//default sql buffer
#ifndef DB_FALLBACK_BLOCKSIZE
#define DB_FALLBACK_BLOCKSIZE      4096
#endif

//internal: do and return the math and ensure it gets realloc'd
static size_t _st_mysql_realloc(void **oblocks, size_t len){
    void *nblocks;
    size_t nlen;
    size_t block_size = 0;
    //round up to standard block sizes
#ifdef HAVE_GETPAGESIZE
    block_size = getpagesize();
#elif defined(_SC_PAGESIZE)
    block_size = sysconf(_SC_PAGESIZE);
#elif defined(_SC_PAGE_SIZE)
    block_size = sysconf(_SC_PAGE_SIZE);
#else
    block_size = DB_FALLBACK_BLOCKSIZE;
#endif
    nlen = (((len - 1) / block_size) + 1) * block_size;
    //keep trying till we get it
    while((nblocks = realloc(*oblocks, nlen)) == NULL) sleep(1);
    *oblocks = nblocks;
    return nlen;
}

//this is the safety check used to make sure there's always enough memory
#define MYSQL_SAFE(blocks, size, len) if((size) >= len) len = _st_mysql_realloc((void**)&(blocks),(size + 1));

typedef list< string > RESDataList;
typedef list< int > ExecList;

enum SQL_TYPE{
    SQL_TYPE_UNKNOW = 0x00,
    SQL_TYPE_SELECT,
    SQL_TYPE_INSERT,
    SQL_TYPE_REPLACE,
    SQL_TYPE_UPDATE,
    SQL_TYPE_DELETE,
};

class DBConnection;
//the follow class use as database connection spool
class DBSpool
{
    public:
        DBSpool();
        ~DBSpool();
        /*
          This function is used to create a legal SQL string that you can use in a SQL statement
          The string pointed to by from must be length bytes long.
          You must allocate the to buffer to be at least length*2+1 bytes long
        */
        unsigned long EscapeString(char* pTo, char* pFrom, unsigned long ulFromLen);
        /*
          set connection counter and the connection timeout,
          shCount should be under DB_MAX_CONNECT and the iTimeout using in microseconds
        */
        bool SetConnection(short shCount, int iTimeout = DB_RECONNECT_TIMEOUT);
        //set database connection parameter, all the connection should be use the same
        bool SetDBparm(const string& strHost, short shPort, const string& strDBname,
                        const string& strUser, const string& strPasswd);
        bool SetDBparm(const char* pcHost, short shPort, const char* pcDBname,
                        const char* pcUser, const char* pcPasswd);
        //connect to database for all connection
        bool Connect();
        //disconnecct all the connection from database
        bool DisConnect();
        /*
          execute the specify sql, the result will be store at MYSQL_RES*.
          the follow sql may be returns result (SELECT, SHOW, DESCRIBE, EXPLAIN).
          at the same time the associate DBConnection will be locked.
          so... after execute the specify sql, caller should call the
          ReleaseConnection(shIdentifier) to release the DBConnection.
          iRelt returns the affect rows by execute strSQL.
          SQL_TYPE_UNKNOW while execute false, otherwise execute success.
        */
        int ExecuteSQL(const string& strSQL, MYSQL_RES** res, short& shIdentifier, int& iRelt, unsigned long long* iInsertId = NULL);
        int ExecuteSQL(const char* pcSQL, MYSQL_RES** res, short& shIdentifier, int& iRelt, unsigned long long* iInsertId = NULL);
        //release the specify DBConnection;
        bool ReleaseConnection(short shIdentifier);
        /*
          execute the specify sql, the result will be store at RESDataList*.
          the follow sql may be returns result (SELECT, SHOW, DESCRIBE, EXPLAIN).
          difference from ExecuteSQL(const string& strSQL, MYSQL_RES** res, short& shIdentifier),
          the DBConnection will be unlocked at once, and caller can NOT call
          ReleaseConnection(shIdentifier) to release the DBConnection.
          iRelt returns the affect rows by execute strSQL.
          SQL_TYPE_UNKNOW while execute false, otherwise execute success.
        */
        int ExecuteSQL(const string& strSQL, RESDataList* res, int& iRelt, unsigned long long* iInsertId = NULL);
        int ExecuteSQL(const char* pcSQL, RESDataList* res, int& iRelt, unsigned long long* iInsertId = NULL);
        //lock
        void Lock();
        //unlock
        void Unlock();

    protected:
        //lock
        void Lock(pthread_mutex_t* pthMutex);
        //unlock
        void Unlock(pthread_mutex_t* pthMutex);

    protected:
        short m_shCount, m_shPort;
        int m_iTimeout;
        string m_strHost, m_strDBname, m_strUser, m_strPasswd;
        ExecList m_IdleList;
        pthread_mutex_t m_thMutex, m_thExecMutex, m_thIdleMutex;
        pthread_cond_t m_thCond;
        MYSQL* m_pSQLConn;
        DBConnection* m_pDBConnection;
};

//the follow class use for connection database
class DBConnection
{
    public:
        DBConnection();
        ~DBConnection();
        //set the connection identifier
        bool SetIdentifier(short shIdentifier);
        //get the connection identifier
        short GetIdentifier();
        //set database connection parameter
        bool SetDBparm(const string& strHost, short shPort, const string& strDBname,
                        const string& strUser, const string& strPasswd);
        bool SetDBparm(const char* pcHost, short shPort, const char* pcDBname,
                        const char* pcUser, const char* pcPasswd);
        //connect to database
        bool Connect();
        //disconnecct from database
        bool DisConnect();
        /*
          execute the specify sql, return SQL_TYPE_UNKNOW while execute false, otherwise execute success.
          the result will be store at MYSQL_RES*, iRelt returns the affect rows by execute strSQL.
        */
        int ExecuteSQL(const string& strSQL, MYSQL_RES** res, int& iRelt, unsigned long long* iInsertId = NULL);
        int ExecuteSQL(const char* pcSQL, MYSQL_RES** res, int& iRelt, unsigned long long* iInsertId = NULL);
        //clear the MYSQL_RES*
        void RestoreRes();
        //return true if the connection is idle
        bool TestIdle();
        //lock
        void Lock();
        //unlock
        void Unlock();

    protected:
        //analyse the sql type, returns should be define in SQL_TYPE
        int SQLtype(const string& strSQL);

    protected:
        short m_shPort, m_shIdentifier;
        string m_strHost, m_strDBname, m_strUser, m_strPasswd;
        pthread_mutex_t m_thMutex;
        MYSQL* m_pSQLConn;
        MYSQL_RES* m_pSQLRes;

    private:
        bool m_bIdle;
};
#endif
