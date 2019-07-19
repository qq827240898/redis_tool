#ifndef  __CHIREDIS__
#define  __CHIREDIS__

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include "biostream.h"
using namespace std;

#include "hiredis.h"

#ifdef WIN32
#include <WinSock2.h>
#pragma comment(lib, "hiredis.lib")
#pragma comment(lib, "WS2_32.lib")
// #else
// #include <hiredis/hiredis.h>
#endif

template<class T>
std::string Type2String(const T& src)
{
	std::stringstream i;
	i << src;

	return i.str();
}

enum DATATYPE
{
	STRINGDATA    = 1,
	HASHDATA      = 2,
	LISTDATA      = 3,
	SETDATA       = 4,
	SORTEDSETDATA = 5,
};


#define	LIBMEMCACHED_DEFAULT_EXP_TIME	0
#define LIBMEMCACHED_DEFAULT_KEYLEN		256
#define LIBMEMCACHED_DEFAULT_VALUELEN	1048320  //1024*1024-LIBMEMCACHED_DEFAULT_KEYLEN
#define LIBMEMCACHED_DEFBUFFER_SIZE     0//开辟最大的内存区

#define LONGLONGLEN						20//long long类型的十进制最大长度

enum _ORDER
{
	DESC = 0,
	ASC = 1,
};

class CCHiredis
{
public:
	CCHiredis(void);
	~CCHiredis(void);

	bool HiConnect(string ip, int port, struct timeval tv,string auth="");
	bool HiConnect(string ip, int port, string auth="");//建立连接、验证密码
	bool HiRedisCommand(string command);	//执行命令

	std::string GetReturnString(){ return m_szString;}
	std::vector<string>& GetReturnArry(){ return m_userActive; }
	long long  GetReturnNumber(){return m_Integer;}

private:
	bool set(const std::string& key, const char* value, unsigned int valuelen, time_t expiration);
	bool replace(const std::string& key, const char* value, unsigned int valuelen, time_t expiration);
	char* get(const std::string& key, unsigned int& valuelen);
	char* hashget(const std::string& hashkey, const std::string &field, unsigned int& valuelen);
	bool del(const std::string& key);

public:
	bool setEver(const std::string& key, const char* value, unsigned int valuelen);
	bool replaceEver(const std::string& key, const char* value, unsigned int valuelen);


	//设置key的过期时间
	bool Set_KeyExpiration(const std::string &key, time_t expiration);

	long long Exist_Key(const std::string &key);

	//hash
	bool SetHash(int hash);

	template<typename T>
	bool Set_HashData(const std::string &hashkey, const std::string &field, const T &value, int bsize = LIBMEMCACHED_DEFBUFFER_SIZE);
	bool Set_HashDataInt(const std::string &hashkey, const std::string &field, int value);
	bool Set_HashDataString(const std::string &hashkey, const std::string &field, const std::string &value);
	template<class T>
	bool Get_HashData(const std::string& hashkey, const std::string &field, T& value);
	bool Get_HashDataString(const std::string& hashkey, const std::string &field, std::string &value);
	long long Set_HashFieldAddNum(const std::string &hashkey, const std::string &field, long long num);


	//sortedset
	long long Set_ZsetData(const std::string &key, long long score, const std::string &member);
	//可变参数列表格式：int，string，num是int和string的总个数，目前只支持int，不支持long long
	long long Set_ZsetMultiData(const std::string &key, int num, ...);
	long long Get_ZsetMemberScore(const std::string &key, const std::string &member);
	long long Get_ZsetRangeCount(const std::string &key, long long min, long long max);
	long long Set_ZsetMemberAddNum(const std::string &key, long long num, const std::string &member);
	//start是开始的下标，end是结束的下标，非score，end可以用负数，-1表示最后一个，-2表示倒数第二个,默认升序排列
	bool Get_ZsetRangeByIndex(const std::string &key, int start, int end, std::vector<std::pair<std::string, long long> > &veclist, _ORDER order = ASC);
	//getall为true的时候，不会用到min和max的值，直接获取所有的数据，此时max和min填0
	bool Get_ZsetRangeByScore(const std::string &key, long long min, long long max, std::vector<std::pair<std::string, long long> > &veclist, bool containmin = true, bool containmax = true, bool getall = false, _ORDER order = ASC);
	//返回member的排名，从0开始的
	long long Get_ZsetMemberRank(const std::string &key, const std::string &member, _ORDER order = ASC);
	long long Del_ZsetMember(const std::string &key, int cnt, ...);

	// string
	bool Set_String(const std::string& key, const std::string& value, time_t expiration = LIBMEMCACHED_DEFAULT_EXP_TIME);

	template<class T>
	bool Set_StringData(const std::string& key, const T& value, time_t expiration = LIBMEMCACHED_DEFAULT_EXP_TIME, int bsize = LIBMEMCACHED_DEFBUFFER_SIZE);

	// replace
	bool Replace_String(const std::string& key, const std::string& value, time_t expiration = LIBMEMCACHED_DEFAULT_EXP_TIME);

	template<class T>
	bool Replace_Data(const std::string& key, const T& value, time_t expiration = LIBMEMCACHED_DEFAULT_EXP_TIME, int bsize = LIBMEMCACHED_DEFBUFFER_SIZE);

	// get
	bool Get_String(const std::string& key, std::string& value);

	template<class T>
	bool Get_StringData(const std::string& key, T& value);


	// delete
	bool Del(const std::string& key);

	void HiRedisFree();										//释放连接上下文
private:
	redisContext* HiRedisConnect(string ip, int port, struct timeval tv);	//建立连接
	redisContext* HiRedisConnect(string ip, int port);		//建立连接
	bool HiAuthPwd(string strPwd);							//密码校验
	
	void HifreeRyObj(void* reply);							//释放命令应答资源
	void HiParseReply(redisReply* ry);						//解析命令结果
	redisContext* GetRedisContent(){ return m_pContent; }	//返回redis连接上下文对象
	bool CheckConnect();
	
private:
	bool InitSocket();

	bool agmem_checkkey(const std::string &key);
	bool agmem_checkvalue(unsigned int valuelen);


private:
	//redis server information
	string			m_cmdstr;
	string			m_redisIP;
	int				m_redisPort;
	string			m_redisAuth;
	struct timeval  m_tv;
	redisContext*	m_pContent;//redis连接上下文指针
	bool			m_bCommand;//连接标识

	std::string     m_szString;

	//std::vector<pair<string,string> >m_userActive;
	std::vector<string> m_userActive;

	long long		m_Integer;
};

template<typename T>
bool CCHiredis::Set_HashData(const std::string &hashkey, const std::string &field, const T &value, int bsize)
{
	if (!agmem_checkkey(hashkey))	return false;
	if (!agmem_checkkey(field))		return false;

	int bufsize = 0;
	if (bsize == LIBMEMCACHED_DEFBUFFER_SIZE)
	{
		bufsize = sizeof(T) + 1;
	}
	else
	{
		bufsize = bsize + 1;
	}

	char *buffer = new char[bufsize];
	bostream bos;
	bos.attach(buffer, bufsize);

	bool flag = false;

	try
	{
		bos << value;
		if (agmem_checkvalue(bos.length()))
		{
			if (!CheckConnect())
			{
				return false;
			}

			const char *v[4];
			size_t vlen[4];

			v[0] = "HSET";
			vlen[0] = strlen("HSET");

			v[1] = hashkey.c_str();
			vlen[1] = strlen(hashkey.c_str());

			v[2] = field.c_str();
			vlen[2] = strlen(field.c_str());

			v[3] = buffer;
			vlen[3] = bos.length();


			redisReply* r = (redisReply *)redisCommandArgv(m_pContent, sizeof(v) / sizeof(v[0]), v, vlen);
			if (NULL == r)
			{
				HiRedisFree();//命令返回NULL，出现严重错误，释放连接
				return false;
			}
			else
			{
				HiParseReply(r);
				HifreeRyObj(r);
			}
			flag = true;
		}
	}
	catch (...)
	{
		fprintf(stderr, "CCHiredis::Set_Data catch bos err === key=%s", m_cmdstr.c_str());
		//Log_Text_Format(LOGLEVEL_ERROR, "CCHiredis::Set_Data catch bos err === key=%s", key.c_str());
	}

	delete[] buffer;
	return flag;
}


template<class T>
bool CCHiredis::Set_StringData(const std::string& key, const T& value, time_t expiration, int bsize)
{
	if (!agmem_checkkey(key))	return false;
	int bufsize = 0;
	if (bsize == LIBMEMCACHED_DEFBUFFER_SIZE)
	{
		bufsize = sizeof(T) + 1;
	}
	else
	{
		bufsize = bsize + 1;
	}

	char *buffer = new char[bufsize];
	bostream bos;
	bos.attach(buffer, bufsize);

	bool flag = false;
	try
	{
		bos << value;
		if (agmem_checkvalue(bos.length()))
		{
			flag = set(key, buffer, bos.length(), expiration);

		}
	}
	catch (...)
	{
		fprintf(stderr, "CCHiredis::Set_Data catch bos err === key=%s", key.c_str());
		//Log_Text_Format(LOGLEVEL_ERROR, "CCHiredis::Set_Data catch bos err === key=%s", key.c_str());
	}

	delete[] buffer;
	return flag;
}



template<class T>
bool CCHiredis::Replace_Data(const std::string& key, const T& value, time_t expiration, int bsize)
{
	if (!agmem_checkkey(key))	return false;
	int bufsize = 0;
	if (bsize == LIBMEMCACHED_DEFBUFFER_SIZE)
	{
		bufsize = sizeof(T)+1;
	}
	else
	{
		bufsize = bsize;
	}

	char *buffer = new char[bufsize];
	//char buffer[LIBMEMCACHED_MAX_BUFFLEN];
	bostream bos;
	bos.attach(buffer, bufsize);

	bool flag = false;
	try
	{
		bos << value;

		if (agmem_checkvalue(bos.length()))
		{
			flag = replace(key, buffer, bos.length(), expiration);
		}
	}
	catch (...)
	{
		fprintf(stderr, "CCHiredis::Replace_Data catch bos err\n");
	}

	delete[]buffer;
	return flag;
}


template<class T>
bool CCHiredis::Get_StringData(const std::string& key, T& value)
{
	if (!agmem_checkkey(key))	return false;

	unsigned int valuelen = 0;
	char*		 valuedata = get(key, valuelen);
	if (0 == valuedata)			return false;

	bistream bis;
	bis.attach(valuedata, valuelen);

	try
	{
		bis >> value;
	}
	catch (...)
	{
		fprintf(stderr, "CCHiredis::Get_Data catch bis err\n");
	}

	free(valuedata);

	return true;
}

template<class T>
bool CCHiredis::Get_HashData(const std::string& hashkey, const std::string &field, T& value)
{
	if (!agmem_checkkey(hashkey))	return false;
	if (!agmem_checkkey(field))		return false;

	unsigned int valuelen = 0;
	char* valuedata = hashget(hashkey, field, valuelen);

	if (0 == valuedata)			return false;

	bistream bis;
	bis.attach(valuedata, valuelen);

	try
	{
		bis >> value;
	}
	catch (...)
	{
		fprintf(stderr, "CCHiredis::Get_HashData catch bis err\n");
	}

	free(valuedata);

	return true;
}

#endif