/*
 * Date         : 2004-09-22
 * Author       : Keqin Su
 * File         : DBSpool.cpp
 * Description  : Class for database connection spool
 */

#include "DBSpool.hpp"

#include <common/LogManager.h>

//the follow class use as database connection spool
DBSpool::DBSpool()
{
    m_shCount = DB_MAX_CONNECT;
    m_shPort = 0;
    m_iTimeout = DB_RECONNECT_TIMEOUT;
    m_strHost = "";
    m_strDBname = "";
    m_strUser = "";
    m_strPasswd = "";
    pthread_mutex_init(&m_thMutex, NULL);
    pthread_mutex_init(&m_thExecMutex, NULL);
    pthread_mutex_init(&m_thIdleMutex, NULL);
    pthread_cond_init(&m_thCond, NULL);
    //m_pSQLConn = NULL;
    m_pDBConnection = NULL;
}

DBSpool::~DBSpool()
{
    pthread_mutex_destroy(&m_thMutex);
    pthread_mutex_destroy(&m_thExecMutex);
    pthread_mutex_destroy(&m_thIdleMutex);
    pthread_cond_destroy(&m_thCond);
    DisConnect();
}

unsigned long DBSpool::EscapeString(char* pTo, char* pFrom, unsigned long ulFromLen)
{
    /*
    if (!m_pSQLConn){
        gvLog(LOG_ALERT, "mysql(DBSpool): call Connect() at first!");
        return 0;
    }
    int iTimes = DB_RECONNECT_TIMES;
    bool bTest = true;
    while (mysql_ping(m_pSQLConn)!=0 && iTimes>0){
        bTest = false;
        gvLog(LOG_ALERT, "mysql(DBSpool): connection to database lost, try to connect remain %d times.", iTimes--);
        if (mysql_real_connect(m_pSQLConn, m_strHost.c_str(), m_strUser.c_str(), m_strPasswd.c_str(),
                                m_strDBname.c_str(), m_shPort, NULL, 0) == NULL){
            gvLog(LOG_ALERT, "mysql(DBSpool): connection to database [%s:%d(%s)] failed: %s", m_strHost.c_str(),
                    m_shPort, m_strDBname.c_str(), mysql_error(m_pSQLConn));
        }else{
           bTest = true;
           break;
        }
        usleep(DB_RECONNECT_TIMEOUT);
    }
    if (bTest){
        return mysql_real_escape_string(m_pSQLConn, pTo, pFrom, ulFromLen);
    }
    */
    mysql_escape_string(pTo, pFrom, ulFromLen);
    return 0;
}

bool DBSpool::SetConnection(short shCount, int iTimeout)
{
    if (shCount>=1 && shCount<=DB_MAX_CONNECT){
        m_shCount = shCount;
        m_iTimeout = iTimeout;
        return true;
    }
    return false;
}

bool DBSpool::SetDBparm(const string& strHost, short shPort, const string& strDBname,
                        const string& strUser, const string& strPasswd)
{
    m_shPort = shPort;
    m_strHost = strHost;
    m_strDBname = strDBname;
    m_strUser = strUser;
    m_strPasswd = strPasswd;
    return true;
}

bool DBSpool::SetDBparm(const char* pcHost, short shPort, const char* pcDBname,
                        const char* pcUser, const char* pcPasswd)
{
    string strHost = pcHost;
    string strDBname = pcDBname;
    string strUser = pcUser;
    string strPasswd = pcPasswd;
    return SetDBparm(strHost, shPort, strDBname, strUser, strPasswd);
}

bool DBSpool::Connect()
{
    if (/*m_pSQLConn || */m_pDBConnection){
        DisConnect();
    }
    /*
    m_pSQLConn = mysql_init((MYSQL*)NULL);
    if(m_pSQLConn == NULL){
        gvLog(LOG_ALERT, "mysql(DBSpool): unable to allocate database connection state!");
        return false;
    }
    if (mysql_real_connect(m_pSQLConn, m_strHost.c_str(), m_strUser.c_str(), m_strPasswd.c_str(),
                            m_strDBname.c_str(), m_shPort, NULL, 0) == NULL){
        gvLog(LOG_ALERT, "mysql(DBSpool): connection to database [%s:%d(%s)] failed: %s", m_strHost.c_str(),
                m_shPort, m_strDBname.c_str(), mysql_error(m_pSQLConn));
        mysql_close(m_pSQLConn);
        m_pSQLConn = NULL;
        return false;
    }
    */
//	MYSQL_RES* SQLRes = NULL;
//  int iRelt = 0;
    Lock(&m_thExecMutex);
    m_pDBConnection = new DBConnection[m_shCount];
    if (m_pDBConnection){
        for (int i=0; i<m_shCount; i++){
            m_pDBConnection[i].SetIdentifier(i + 1);
            m_IdleList.push_back(i);
            m_pDBConnection[i].SetDBparm(m_strHost, m_shPort, m_strDBname, m_strUser, m_strPasswd);
            if (!m_pDBConnection[i].Connect()){
                DisConnect();
                Unlock(&m_thExecMutex);
                return false;
            }
/*
#ifdef CLN_UTF8
			m_pDBConnection[i].ExecuteSQL("set names utf8", &SQLRes, iRelt);//���ÿͻ����ַ�
#endif
*/
        }
    }
    Unlock(&m_thExecMutex);
    return true;
}

bool DBSpool::DisConnect()
{
    /*
    if (m_pSQLConn){
        mysql_close(m_pSQLConn);
        m_pSQLConn = NULL;
    }
    */
    if (m_pDBConnection){
        delete[] m_pDBConnection;
        m_pDBConnection = NULL;
    }
    return true;
}

int DBSpool::ExecuteSQL(const string& strSQL, MYSQL_RES** res, short& shIdentifier, int& iRelt, unsigned long long* iInsertId)
{
    if (m_pDBConnection == NULL){
        return SQL_TYPE_UNKNOW;
    }
    int i = -1, iState = 0;
    timespec tTime;
    timeval tNow;
    while (i == -1){
        gettimeofday(&tNow, NULL);
        tTime.tv_sec = tNow.tv_sec + 1;
        tTime.tv_nsec = 0;
        //lock idle list
        Lock(&m_thExecMutex);
        if (m_IdleList.size() < 1){
            Unlock(&m_thExecMutex);
            //lock condition
            Lock(&m_thIdleMutex);
            iState = pthread_cond_timedwait(&m_thCond, &m_thIdleMutex, &tTime);
            Unlock(&m_thIdleMutex);
            if (iState == 0){
                //lock idle list
                Lock(&m_thExecMutex);
                if (m_IdleList.size() > 0){
                    i = m_IdleList.front();
                    m_IdleList.pop_front();
                    Unlock(&m_thExecMutex);
                    break;
                }else{
                    Unlock(&m_thExecMutex);
                    continue;
                }
            }else if(iState == ETIMEDOUT){
                return SQL_TYPE_UNKNOW;
            }
        }else{
            i = m_IdleList.front();
            m_IdleList.pop_front();
            Unlock(&m_thExecMutex);
        }
    }

    if (i >= 0){
        m_pDBConnection[i].Lock();
        int iRet = m_pDBConnection[i].ExecuteSQL(strSQL, res, iRelt, iInsertId);
        int iSize = 0;

        switch (iRet){
            case SQL_TYPE_SELECT:
                shIdentifier = i + 1;
                break;
            case SQL_TYPE_INSERT:
            case SQL_TYPE_UPDATE:
            case SQL_TYPE_DELETE:
            default:
                shIdentifier = 0;
                m_pDBConnection[i].Unlock();
                //lock idle list
                Lock(&m_thExecMutex);
                m_IdleList.push_back(i);
                iSize = m_IdleList.size();
                Unlock(&m_thExecMutex);
                //broadcast condition
                if (iSize == 1){
                    Lock(&m_thIdleMutex);
                    pthread_cond_broadcast(&m_thCond);
                    Unlock(&m_thIdleMutex);
                }
        }
        return iRet;
    }
    return SQL_TYPE_UNKNOW;
}

int DBSpool::ExecuteSQL(const char* pcSQL, MYSQL_RES** res, short& shIdentifier, int& iRelt, unsigned long long* iInsertId)
{
    string strSQL = pcSQL;
    return ExecuteSQL(strSQL, res, shIdentifier, iRelt, iInsertId);
}

bool DBSpool::ReleaseConnection(short shIdentifier)
{
    if (shIdentifier>=1 && m_shCount<=DB_MAX_CONNECT){
        m_pDBConnection[shIdentifier - 1].RestoreRes();
        m_pDBConnection[shIdentifier - 1].Unlock();
        //lock idle list
        Lock(&m_thExecMutex);
        m_IdleList.push_back(shIdentifier - 1);
        int iSize = m_IdleList.size();
        Unlock(&m_thExecMutex);
        //broadcast condition
        if (iSize == 1){
            Lock(&m_thIdleMutex);
            pthread_cond_broadcast(&m_thCond);
            Unlock(&m_thIdleMutex);
        }
        return true;
    }
    return false;
}

int DBSpool::ExecuteSQL(const string& strSQL, RESDataList* res, int &iRelt, unsigned long long* iInsertId)
{
    MYSQL_RES* SQLRes = NULL;
    short shIdt = 0;
    int iRet = ExecuteSQL(strSQL, &SQLRes, shIdt, iRelt, iInsertId);
    if (iRet == SQL_TYPE_SELECT){
        //MYSQL_FIELD* fields;
        MYSQL_ROW row;
        int irows, ifields;
        string strValue;
        irows = mysql_num_rows(SQLRes);
        if(irows == 0) {
            ReleaseConnection(shIdt);
            return iRet;
        }
        ifields = mysql_num_fields(SQLRes);
        if(ifields == 0){
            ReleaseConnection(shIdt);
            return iRet;
        }
        //fields = mysql_fetch_fields(SQLRes);
        for(int i=0; i<irows; i++){
            if((row = mysql_fetch_row(SQLRes)) == NULL){
                break;
            }
            for(int j=0; j<ifields; j++) {
                if (row[j]){
                    strValue = row[j];
                }else{
                    strValue = "";
                }
                res->push_back(strValue);
            }
        }
        ReleaseConnection(shIdt);
    }
    return iRet;
}

int DBSpool::ExecuteSQL(const char* pcSQL, RESDataList* res, int& iRelt, unsigned long long* iInsertId)
{
    string strSQL = pcSQL;
    return ExecuteSQL(strSQL, res, iRelt, iInsertId);
}

inline void DBSpool::Lock()
{
    pthread_mutex_lock(&m_thMutex);
}

inline void DBSpool::Unlock()
{
    pthread_mutex_unlock(&m_thMutex);
}

inline void DBSpool::Lock(pthread_mutex_t* pthMutex)
{
    pthread_mutex_lock(pthMutex);
}

inline void DBSpool::Unlock(pthread_mutex_t* pthMutex)
{
    pthread_mutex_unlock(pthMutex);
}

//the follow class use for connection database
DBConnection::DBConnection()
{
    m_shPort = 0;
    m_shIdentifier = 0;
    m_strHost = "";
    m_strDBname = "";
    m_strUser = "";
    m_strPasswd = "";
    pthread_mutex_init(&m_thMutex, NULL);
    m_pSQLConn = NULL;
    m_pSQLRes = NULL;
    m_bIdle = true;
}

DBConnection::~DBConnection()
{
    pthread_mutex_destroy(&m_thMutex);
    DisConnect();
}

bool DBConnection::SetIdentifier(short shIdentifier)
{
    if (shIdentifier>=1 && shIdentifier<=DB_MAX_CONNECT){
        m_shIdentifier = shIdentifier;
        return true;
    }
    return false;
}

short DBConnection::GetIdentifier()
{
    return m_shIdentifier;
}

bool DBConnection::SetDBparm(const string& strHost, short shPort, const string& strDBname,
                            const string& strUser, const string& strPasswd)
{
    m_shPort = shPort;
    m_strHost = strHost;
    m_strDBname = strDBname;
    m_strUser = strUser;
    m_strPasswd = strPasswd;
    return true;
}

bool DBConnection::SetDBparm(const char* pcHost, short shPort, const char* pcDBname,
                            const char* pcUser, const char* pcPasswd)
{
    string strHost = pcHost;
    string strDBname = pcDBname;
    string strUser = pcUser;
    string strPasswd = pcPasswd;
    return SetDBparm(strHost, shPort, strDBname, strUser, strPasswd);
}

bool DBConnection::Connect()
{
    if (m_pSQLConn){
        DisConnect();
    }
    m_pSQLConn = mysql_init((MYSQL*)NULL);
    if(m_pSQLConn == NULL) {
        return false;
    }
//    mysql_options(m_pSQLConn, MYSQL_SET_CHARSET_NAME, "utf8");
    mysql_options(m_pSQLConn, MYSQL_SET_CHARSET_NAME, "latin1");
    if (mysql_real_connect(m_pSQLConn, m_strHost.c_str(), m_strUser.c_str(), m_strPasswd.c_str(),
                            m_strDBname.c_str(), m_shPort, NULL, 0) == NULL){
        mysql_close(m_pSQLConn);
        m_pSQLConn = NULL;
        return false;
    }

//#ifdef CLN_UTF8
//    if (m_pSQLConn) {
//        mysql_query(m_pSQLConn, "set names utf8");
//    }
//#endif

    return true;
}

bool DBConnection::DisConnect()
{
    while (!TestIdle()) usleep(100);
    RestoreRes();
    if (m_pSQLConn){
        mysql_close(m_pSQLConn);
        m_pSQLConn = NULL;
    }
    return true;
}

int DBConnection::ExecuteSQL(const string& strSQL, MYSQL_RES** res, int& iRelt, unsigned long long* iInsertId)
{
    if (m_pSQLConn == NULL){
        return SQL_TYPE_UNKNOW;
    }
    int iTimes = DB_RECONNECT_TIMES;
    bool bTest = true;
	bool bHasUnlock=false;
    while (iTimes-- > 0){
    	if( m_pSQLConn != NULL ) {
        	if( mysql_ping(m_pSQLConn) != 0 ) {
                bTest = false;
        		if(!bHasUnlock) Unlock();
        		bHasUnlock = true;
                if (Connect()){
                    bTest = true;
                    break;
                }

        	} else {
        		bTest = true;
        		break;
        	}

    	} else {
            bTest = false;
    		if(!bHasUnlock) Unlock();
    		bHasUnlock = true;
            if (Connect()){
                bTest = true;
                break;
            }
    	}

    	usleep(DB_RECONNECT_TIMEOUT);
    }
	if(bHasUnlock)Lock();
    if (!bTest){
        return SQL_TYPE_UNKNOW;
    }
    if (mysql_query(m_pSQLConn, strSQL.c_str()) != 0){
    	// print error
    	const char* error = mysql_error(m_pSQLConn);
		LogAync(
				LOG_ERR,
				"DBConnection::ExecuteSQL, "
				"[执行SQL语句失败], "
				"error:%s, "
				"sql:%s"
				")",
				NULL!=error ? error : "Unknow",
				strSQL.c_str()
				);
        return SQL_TYPE_UNKNOW;
    }
    int iType = SQLtype(strSQL);
    switch (iType) {
        case SQL_TYPE_SELECT:
            m_pSQLRes = mysql_store_result(m_pSQLConn);
            if (m_pSQLRes==NULL){
            	if( mysql_field_count(m_pSQLConn)!=0 ) {
					iRelt = 0;
            	} else {
            		return SQL_TYPE_UNKNOW;
            	}

            }else{
				*res = m_pSQLRes;
				iRelt = mysql_num_rows(m_pSQLRes);

            }
            break;
        case SQL_TYPE_INSERT: {
        	if( iInsertId != NULL ) {
        		*iInsertId = mysql_insert_id(m_pSQLConn);
        	}
        }
        case SQL_TYPE_REPLACE:
        case SQL_TYPE_UPDATE:
        case SQL_TYPE_DELETE:
        default:
            iRelt = mysql_affected_rows(m_pSQLConn);
            break;
    }
    return iType;
}

int DBConnection::ExecuteSQL(const char* pcSQL, MYSQL_RES** res, int& iRelt, unsigned long long* iInsertId)
{
    string strSQL = pcSQL;
    return ExecuteSQL(strSQL, res, iRelt, iInsertId);
}

void DBConnection::RestoreRes()
{
    if (m_pSQLRes){
        mysql_free_result(m_pSQLRes);
        m_pSQLRes = NULL;
    }
}

inline bool DBConnection::TestIdle()
{
    return m_bIdle;
}

inline void DBConnection::Lock()
{
    m_bIdle = false;
    pthread_mutex_lock(&m_thMutex);
}

inline void DBConnection::Unlock()
{
    pthread_mutex_unlock(&m_thMutex);
    m_bIdle = true;
}

int DBConnection::SQLtype(const string& strSQL)
{
    if(!memcmp(strSQL.c_str(), "SELECT", 6) || !memcmp(strSQL.c_str(), "select", 6)){
        return SQL_TYPE_SELECT;
    }else if(!memcmp(strSQL.c_str(), "INSERT", 6) || !memcmp(strSQL.c_str(), "insert", 6)){
        return SQL_TYPE_INSERT;
    }else if(!memcmp(strSQL.c_str(), "UPDATE", 6) || !memcmp(strSQL.c_str(), "update", 6)){
        return SQL_TYPE_UPDATE;
    }else if(!memcmp(strSQL.c_str(), "DELETE", 6) || !memcmp(strSQL.c_str(), "delete", 6)){
        return SQL_TYPE_DELETE;
    }else if(!memcmp(strSQL.c_str(), "REPLACE", 7) || !memcmp(strSQL.c_str(), "replace", 7)){
        return SQL_TYPE_REPLACE;
    }
    return SQL_TYPE_UNKNOW;
}
