#include "CHiredis.h"
CCHiredis::CCHiredis(void)
:m_pContent(NULL)
,m_bCommand(false)
,m_Integer(0)
{
	InitSocket();
}

CCHiredis::~CCHiredis(void)
{
#ifdef WIN32
	WSACleanup();
#endif
}

void CCHiredis::HiRedisFree()
{
	if (m_pContent)
	{
		redisFree(m_pContent);
		m_pContent = NULL;
	}
	
	m_bCommand = false;
}

void CCHiredis::HifreeRyObj(void* reply)
{
	if (reply)
	{
		freeReplyObject(reply);
		reply = NULL;
	}
}

bool CCHiredis::InitSocket()
{
#ifdef WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,1), &wsaData)==0)
	{
		return false;
	}
#endif
	return true;
}

bool CCHiredis::HiConnect(string ip, int port, string auth)
{
	m_redisIP =ip, m_redisPort = port; m_redisAuth = auth;

	if (!CheckConnect())
	{
		return false;
	}

	return true;
}

//connect redis server
redisContext* CCHiredis::HiRedisConnect(string ip, int port)
{
	if (m_pContent==NULL)
	{
		m_pContent = redisConnect(ip.c_str(), port);
		if (m_pContent==NULL || m_pContent->err)
		{
			HiRedisFree();
			return m_pContent;
		}
	}

	m_bCommand = true;//连接成功标识

	return m_pContent;
}

bool CCHiredis::HiConnect(string ip, int port, struct timeval tv,string auth)
{
	m_redisIP=ip, m_redisPort=port; m_tv=tv; m_redisAuth=auth;

	if (!CheckConnect())
	{
		return false;
	}

	return true;
}

redisContext* CCHiredis::HiRedisConnect(string ip, int port, struct timeval tv)
{
	if (m_pContent==NULL)
	{
		m_pContent = redisConnectWithTimeout(ip.c_str(), port, tv);
		if (m_pContent==NULL || m_pContent->err)
		{
			HiRedisFree();
			return m_pContent;
		}
	}

	m_bCommand = true;//连接成功标识

	return m_pContent;
}

//auth redis password
bool CCHiredis::HiAuthPwd(string strPwd)
{
	redisReply* r =(redisReply*)redisCommand(m_pContent, "AUTH %s", strPwd.c_str());
	if (NULL==r)
	{
		HiRedisFree();//严重错误，释放连接

		HifreeRyObj(r);
		return false;	
	}

	if (r->type == REDIS_REPLY_ERROR)
	{
		HifreeRyObj(r);
		return false;
	}

	if (r->type==REDIS_REPLY_STATUS && (strcmp(r->str,"OK")== 0 || strcmp(r->str, "ok") == 0))
	{
		HifreeRyObj(r);		
	}
	
	return true;
}

bool CCHiredis::CheckConnect()
{
	if (false==m_bCommand)
	{
		m_tv.tv_sec = 3; m_tv.tv_usec = 10;
		if (!(HiRedisConnect(m_redisIP.c_str(), m_redisPort,m_tv) || HiRedisConnect(m_redisIP.c_str(), m_redisPort)))
		//if (HiRedisConnect(m_redisIP.c_str(), m_redisPort))
		{
			return false;
		}

		if (!m_redisAuth.empty())
		{
			if (false==HiAuthPwd(m_redisAuth))//重新密码认证
			{
				return false;
			}
		}
	}

	return true;
}

bool CCHiredis::HiRedisCommand(string command)
{
	if (!CheckConnect())
	{
		return false;
	}

	redisReply* r =(redisReply*)redisCommand(m_pContent, command.c_str());
	if (NULL==r)
	{
		HiRedisFree();//命令返回NULL，出现严重错误，释放连接
		return false;
	}
	else
	{
		HiParseReply(r);
		HifreeRyObj(r);
	}

	return true;
}

void CCHiredis::HiParseReply(redisReply* ry)
{
	if (ry->type == REDIS_REPLY_INTEGER)
	{
		m_Integer = ry->integer;
		m_szString.clear();
	}
	else if (ry->type == REDIS_REPLY_ARRAY)
	{
		m_userActive.clear();
		m_szString.clear();

		for (int i=0;i<ry->elements;i++)
		{
			if (ry->element[i]->type == REDIS_REPLY_INTEGER)
			{
				m_userActive.push_back(Type2String(ry->element[i]->integer));
			}
			else
			{
				m_userActive.push_back(ry->element[i]->str);
			}
		}
	}
	else if (ry->type == REDIS_REPLY_STRING)
	{	
		m_szString = ry->str;
	}
	else if (ry->type == REDIS_REPLY_STATUS)
	{//set 命令到此
		m_szString = ry->str;
	}
	else if (ry->type == REDIS_REPLY_ERROR)
	{
		m_szString.clear();
	}
	else if (ry->type == REDIS_REPLY_NIL)
	{
		m_szString.clear();
	}	
}

bool CCHiredis::set(const std::string& key, const char* value, unsigned int valuelen, time_t expiration)
{
	if (expiration == 0)
		return setEver(key, value, valuelen);


	if (!CheckConnect())
	{
		return false;
	}

	const char *v[5];
	size_t vlen[5];

	v[0] = "SET";
	vlen[0] = strlen("SET");

	v[1] = key.c_str();
	vlen[1] = strlen(key.c_str());

	v[2] = value;
	vlen[2] = valuelen;

	v[3] = "EX";
	vlen[3] = strlen("EX");

	char expirationstr[256] = { 0 };
	sprintf(expirationstr, "%d", expiration);

	v[4] = expirationstr;
	vlen[4] = strlen(expirationstr);


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

	if (m_szString.compare("OK") && m_szString.compare("ok"))
	{
		fprintf(stderr, "CCHiredis::set--HiRedisCommand error:%s\n", key);
		return false;
	}

	return true;
}



bool CCHiredis::replace(const std::string& key, const char* value, unsigned int valuelen, time_t expiration)
{
	if (expiration == 0)
		return replaceEver(key, value, valuelen);


	if (!CheckConnect())
	{
		return false;
	}

	const char *v[6];
	size_t vlen[6];

	v[0] = "SET";
	vlen[0] = strlen("SET");

	v[1] = key.c_str();
	vlen[1] = strlen(key.c_str());

	v[2] = value;
	vlen[2] = valuelen;

	v[3] = "EX";
	vlen[3] = strlen("EX");

	char expirationstr[256] = { 0 };
	sprintf(expirationstr, "%d", expiration);

	v[4] = expirationstr;
	vlen[4] = strlen(expirationstr);

	v[5] = "XX";
	vlen[5] = strlen("XX");


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

	if (m_szString.compare("OK") && m_szString.compare("ok"))
	{
		fprintf(stderr, "CCHiredis::replace--HiRedisCommand error:%s\n", key);
		return false;
	}

	return true;
}


bool CCHiredis::setEver(const std::string& key, const char* value, unsigned int valuelen)
{
	if (!CheckConnect())
	{
		return false;
	}

	const char *v[3];
	size_t vlen[3];

	v[0] = "SET";
	vlen[0] = strlen("SET");
	
	v[1] = key.c_str();
	vlen[1] = strlen(key.c_str());

	v[2] = value;
	vlen[2] = valuelen;


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

	if (m_szString.compare("OK") && m_szString.compare("ok"))
	{
		fprintf(stderr, "CCHiredis::set--HiRedisCommand error:%s\n", key);
		return false;
	}

	return true;
}


bool CCHiredis::replaceEver(const std::string& key, const char* value, unsigned int valuelen)
{
	if (!CheckConnect())
	{
		return false;
	}

	const char *v[4];
	size_t vlen[4];

	v[0] = "SET";
	vlen[0] = strlen("SET");

	v[1] = key.c_str();
	vlen[1] = strlen(key.c_str());

	v[2] = value;
	vlen[2] = valuelen;

	v[3] = "XX";
	vlen[3] = strlen("XX");


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

	if (m_szString.compare("OK") && m_szString.compare("ok"))
	{
		fprintf(stderr, "CCHiredis::replace--HiRedisCommand error:%s\n", key);
		return false;
	}

	return true;
}


bool CCHiredis::Set_KeyExpiration(const std::string &key, time_t expiration)
{
	bool ret = false;
	if (!agmem_checkkey(key))	return false;

	char buf[LONGLONGLEN];
	
	sprintf(buf, "%lld", expiration);

	m_cmdstr.clear();
	m_cmdstr = "EXPIRE " + key + " " + std::string(buf);
	if (HiRedisCommand(m_cmdstr))
	{
		if (m_Integer == 1)
		{
			ret = true;
		}
	}

	return ret;
}

long long CCHiredis::Exist_Key(const std::string &key)
{
	if (!agmem_checkkey(key))	return -1;

	m_cmdstr.clear();
	m_cmdstr = "EXISTS " + key;

	if (!HiRedisCommand(m_cmdstr))
	{
		return -1;
	}

	return m_Integer;
}

char* CCHiredis::get(const std::string& key, unsigned int& valuelen)
{
	char* value = 0;

	if (!CheckConnect())
	{
		return false;
	}

	string command = string("GET ") + key;

	redisReply* r = (redisReply*)redisCommand(m_pContent, command.c_str());
	if (NULL == r)
	{
		HiRedisFree();//命令返回NULL，出现严重错误，释放连接
		return false;
	}
	else
	{
		if (r->type == REDIS_REPLY_STRING)
		{
			valuelen = r->len;
			value = (char *)malloc(valuelen);
			memcpy(value, r->str, r->len);
		}
		HifreeRyObj(r);
		return value;
	}
}



char* CCHiredis::hashget(const std::string& hashkey, const std::string &field, unsigned int& valuelen)
{
	char* value = 0;

	if (!CheckConnect())
	{
		return false;
	}

	string command = string("HGET ") + hashkey + " " + field;

	redisReply* r = (redisReply*)redisCommand(m_pContent, command.c_str());
	if (NULL == r)
	{
		HiRedisFree();//命令返回NULL，出现严重错误，释放连接
		return false;
	}
	else
	{
		if (r->type == REDIS_REPLY_STRING)
		{
			valuelen = r->len;
			value = (char *)malloc(valuelen);
			memcpy(value, r->str, r->len);
		}
		HifreeRyObj(r);
		return value;
	}
}

bool CCHiredis::del(const std::string& key)
{
	if (!CheckConnect())
	{
		return false;
	}

	string command = string("DEL ") + key;

	redisReply* r = (redisReply*)redisCommand(m_pContent, command.c_str());
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

	return m_Integer>0;
}


bool CCHiredis::SetHash(int hash)
{
	// memcached_behavior_set(pstMCache, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED, 1);
	return true;
}

bool CCHiredis::Set_HashDataInt(const std::string &hashkey, const std::string &field, int value)
{
	char tmp[LONGLONGLEN] = { 0 };
	sprintf(tmp, "%d", value);
	std::string strtmp(tmp);
	return Set_HashDataString(hashkey, field, strtmp);
}

bool CCHiredis::Set_HashDataString(const std::string &hashkey, const std::string &field, const std::string &value)
{
	if (!agmem_checkkey(hashkey))	return false;
	if (!agmem_checkkey(field))		return false;

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

	v[3] = value.c_str();
	vlen[3] = strlen(value.c_str());


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
		return true;
	}
}

bool CCHiredis::Get_HashDataString(const std::string& hashkey, const std::string &field, std::string &value)
{
	if (!agmem_checkkey(hashkey))	return false;
	if (!agmem_checkkey(field))	return false;
	if (!CheckConnect())
	{
		return false;
	}

	string command = string("HGET ") + hashkey + " " + field;

	redisReply* r = (redisReply*)redisCommand(m_pContent, command.c_str());
	if (NULL == r)
	{
		HiRedisFree();//命令返回NULL，出现严重错误，释放连接
		return false;
	}
	else
	{
		if (r->type == REDIS_REPLY_STRING)
		{
			value = r->str;
		}
		HifreeRyObj(r);
		return true;
	}
}

long long CCHiredis::Set_HashFieldAddNum(const std::string &hashkey, const std::string &field, long long num)
{
	if (!agmem_checkkey(hashkey))	return -1;
	if (!agmem_checkkey(field))	return -1;

	char buf[LONGLONGLEN];
	sprintf(buf, "%lld", num);

	m_cmdstr.clear();
	m_cmdstr = "HINCRBY " + hashkey + " " + field + " " + std::string(buf);

	if (!HiRedisCommand(m_cmdstr))
	{
		return -1;
	}
	else
	{
		return m_Integer;
	}
}




long long CCHiredis::Set_ZsetData(const std::string &key, long long score, const std::string &member)
{
	if (!agmem_checkkey(key))	return -1;
	if (!agmem_checkkey(member))	return -1;

	char buf[LONGLONGLEN];
	sprintf(buf, "%lld", score);


	m_cmdstr.clear();
	m_cmdstr = "ZADD " + key + " " + string(buf) +  " " + member;

	if (!HiRedisCommand(m_cmdstr))
	{
		return -1;
	}

	return m_Integer;
}

long long CCHiredis::Set_ZsetMultiData(const std::string &key, int num, ...)
{
	if (!agmem_checkkey(key))	return -1;
	char buf[LONGLONGLEN];
	char memberbuf[128];
	std::string tmpstr;
	m_cmdstr.clear();
	m_cmdstr = "ZADD " + key;
	va_list args;
	va_start(args, num);
	for (int i = 0; i < num / 2; i++)
	{
		memset(buf, 0, sizeof(char) * LONGLONGLEN);
		sprintf(buf, "%d", va_arg(args, int));
		m_cmdstr.append(" ");
		m_cmdstr.append(buf);
		m_cmdstr.append(" ");
		m_cmdstr.append(va_arg(args, char*));
	}
	va_end(args);

	if (!HiRedisCommand(m_cmdstr))
	{
		return -1;
	}

	return m_Integer;
}

long long CCHiredis::Get_ZsetMemberScore(const std::string &key, const std::string &member)
{
	if (!agmem_checkkey(key))	return -1;
	if (!agmem_checkkey(member))	return -1;

	if (!CheckConnect())
	{
		return -1;
	}

	m_cmdstr.clear();
	m_cmdstr = "ZSCORE " + key + " " + member;

	if (!HiRedisCommand(m_cmdstr))
	{
		return -1;
	}

	return atol(m_szString.c_str());
}

long long CCHiredis::Get_ZsetRangeCount(const std::string &key, long long min, long long max)
{
	if (!agmem_checkkey(key))	return -1;

	if (!CheckConnect())
	{
		return -1;
	}

	char buf1[LONGLONGLEN];
	char buf2[LONGLONGLEN];

	sprintf(buf1, "%lld", min);
	sprintf(buf2, "%lld", max);


	m_cmdstr.clear();
	m_cmdstr = "ZCOUNT " + key + " " + std::string(buf1) + " " + std::string(buf2);

	if (!HiRedisCommand(m_cmdstr))
	{
		return -1;
	}

	return m_Integer;
}

long long CCHiredis::Set_ZsetMemberAddNum(const std::string &key, long long num, const std::string &member)
{
	if (!agmem_checkkey(key))	return -1;
	if (!agmem_checkkey(member))	return -1;

	char buf[LONGLONGLEN];
	sprintf(buf, "%lld", num);

	m_cmdstr.clear();
	m_cmdstr = "ZINCRBY " + key + " " + std::string(buf) + " " + member;

	if (!HiRedisCommand(m_cmdstr))
	{
		return -1;
	}

	return atol(m_szString.c_str());
}

bool CCHiredis::Get_ZsetRangeByIndex(const std::string &key, int start, int end, std::vector<std::pair<std::string, long long> > &veclist, _ORDER order/* = ASC*/)
{
	if (!agmem_checkkey(key))	return false;

	if (!CheckConnect())
	{
		return false;
	}

	char buf1[LONGLONGLEN];
	char buf2[LONGLONGLEN];

	sprintf(buf1, "%d", start);
	sprintf(buf2, "%d", end);

	m_cmdstr.clear();
	if (order == ASC)
	{
		m_cmdstr = "ZRANGE " + key + " " + std::string(buf1) + " " + std::string(buf2) + " " + std::string("WITHSCORES");
	}
	else
	{
		m_cmdstr = "ZREVRANGE " + key + " " + std::string(buf1) + " " + std::string(buf2) + " " + std::string("WITHSCORES");
	}
	
	redisReply* r = (redisReply*)redisCommand(m_pContent, m_cmdstr.c_str());
	if (NULL == r)
	{
		HiRedisFree();//命令返回NULL，出现严重错误，释放连接
		return false;
	}
	else
	{
		HiParseReply(r);
		std::pair<std::string, long long> tmppair;
		for (int i = 0; i < r->elements; )
		{
			tmppair = make_pair(m_userActive[i], atoi(m_userActive[i + 1].c_str()));
			veclist.push_back(tmppair);
			i += 2;
		}
		HifreeRyObj(r);
		return true;
	}
}

bool CCHiredis::Get_ZsetRangeByScore(const std::string &key, long long min, long long max, std::vector<std::pair<std::string, long long> > &veclist, bool containmin /*= true*/, bool containmax /*= true*/, bool getall /*= false*/, _ORDER order /*= ASC*/)
{
	if (!agmem_checkkey(key))	return false;

	if (!CheckConnect())
	{
		return false;
	}

	char buf1[LONGLONGLEN];
	char buf2[LONGLONGLEN];

	sprintf(buf1, "%lld", min);
	sprintf(buf2, "%lld", max);

	m_cmdstr.clear();
	if (order == ASC)
	{
		m_cmdstr = "ZRANGEBYSCORE " + key + " ";
	}
	else
	{
		m_cmdstr = "ZREVRANGEBYSCORE " + key + " ";
	}

	if (getall)
	{
		m_cmdstr.append("-inf +inf WITHSCORES");
	}
	else
	{
		if (order == ASC)
		{
			if (!containmin)
			{
				m_cmdstr.append("(");
			}
			m_cmdstr.append(std::string(buf1));
			m_cmdstr.append(" ");
			if (!containmax)
			{
				m_cmdstr.append("(");
			}
			m_cmdstr.append(std::string(buf2));
			m_cmdstr.append(" WITHSCORES");
		}
		else
		{
			if (!containmin)
			{
				m_cmdstr.append("(");
			}
			m_cmdstr.append(std::string(buf2));
			m_cmdstr.append(" ");
			if (!containmax)
			{
				m_cmdstr.append("(");
			}
			m_cmdstr.append(std::string(buf1));
			m_cmdstr.append(" WITHSCORES");
		}
	}
	
	redisReply* r = (redisReply*)redisCommand(m_pContent, m_cmdstr.c_str());
	if (NULL == r)
	{
		HiRedisFree();//命令返回NULL，出现严重错误，释放连接
		return false;
	}
	else
	{
		HiParseReply(r);
		std::pair<std::string, long long> tmppair;
		for (int i = 0; i < r->elements; )
		{
			tmppair = make_pair(m_userActive[i], atoi(m_userActive[i + 1].c_str()));
			veclist.push_back(tmppair);
			i += 2;
		}
		HifreeRyObj(r);
		return true;
	}
}

long long CCHiredis::Get_ZsetMemberRank(const std::string &key, const std::string &member, _ORDER order /*= ASC*/)
{
	if (!agmem_checkkey(key))	return -1;
	if (!agmem_checkkey(member))	return -1;

	m_cmdstr.clear();
	if (order == ASC)
	{
		m_cmdstr = "ZRANK " + key + " " + member;
	}
	else
	{
		m_cmdstr = "ZREVRANK " + key + " " + member;
	}
	
	
	if (!HiRedisCommand(m_cmdstr))
	{
		return -1;
	}

	return m_Integer;
}

long long CCHiredis::Del_ZsetMember(const std::string &key, int cnt, ...)
{
	if (!agmem_checkkey(key))	return -1;

	m_cmdstr.clear();
	m_cmdstr = "ZREM " + key;

	va_list args;
	va_start(args, cnt);

	for (int i = 0; i < cnt; i++)
	{
		m_cmdstr.append(" ");
		m_cmdstr.append(va_arg(args, char *));
	}
	va_end(args);

	if (!HiRedisCommand(m_cmdstr))
	{
		return -1;
	}

	return m_Integer;
}

// set
bool CCHiredis::Set_String(const std::string& key, const std::string& value, time_t expiration)
{
	if (!agmem_checkkey(key))	return false;
	if (!agmem_checkvalue(value.length()))	return false;


	m_cmdstr.clear();

	if (expiration == 0)
	{
		m_cmdstr = string("SET ") + key + " " + value;
	}
	else
	{
		char expirationstr[256] = { 0 };
		sprintf(expirationstr, "%d", expiration);
		m_cmdstr = string("SET ") + key + " " + value + " " + "EX " + expirationstr;
	}


	if (!HiRedisCommand(m_cmdstr))
		return false;

	if (m_szString.compare("OK") && m_szString.compare("ok"))
	{
		fprintf(stderr, "CCHiredis::Set_String--HiRedisCommand error:%s\n", m_cmdstr);
		return false;
	}

	return true;
}



bool CCHiredis::Replace_String(const std::string& key, const std::string& value, time_t expiration)
{
	if (!agmem_checkkey(key))	return false;
	if (!agmem_checkvalue(value.length()))	return false;


	m_cmdstr.clear();

	if (expiration == 0)
	{
		m_cmdstr = string("SET ") + key + " " + value + " " + "XX";
	}
	else
	{
		char expirationstr[256] = { 0 };
		sprintf(expirationstr, "%d", expiration);
		m_cmdstr = string("SET ") + key + " " + value + " " + "EX " + expirationstr + " " + "XX";
	}

	if (!HiRedisCommand(m_cmdstr))
		return false;

	if (m_szString.compare("OK") && m_szString.compare("ok"))
	{
		fprintf(stderr, "CCHiredis::Replace_String--HiRedisCommand error:%s\n", m_cmdstr);
		return false;
	}

	return true;
}



bool CCHiredis::Get_String(const std::string& key, std::string& value)
{
	if (!agmem_checkkey(key))	return false;

	if (!CheckConnect())
	{
		return false;
	}

	m_cmdstr = string("GET ") + key;

	redisReply* r = (redisReply*)redisCommand(m_pContent, m_cmdstr.c_str());
	if (NULL == r)
	{
		HiRedisFree();//命令返回NULL，出现严重错误，释放连接
		return false;
	}
	else
	{
		HiParseReply(r);

		if (r->type == REDIS_REPLY_STRING)
		{
			value = m_szString;
			HifreeRyObj(r);
			return true;
		}
		else
		{
			HifreeRyObj(r);
			return false;
		}
	}
}


bool CCHiredis::Del(const std::string& key)
{
	if (!agmem_checkkey(key))	return false;

	return del(key);
}


bool CCHiredis::agmem_checkkey(const std::string& key)
{
	if (key.length() <= 0 || key.length() >= LIBMEMCACHED_DEFAULT_KEYLEN)	return false;

	for (std::string::const_iterator it = key.begin(); it != key.end(); ++it)
	{
		unsigned char c = (unsigned char)*it;
		if (c < 0x21 || c == 0x7f)	return false;
	}

	return true;
}

bool CCHiredis::agmem_checkvalue(unsigned int valuelen)
{
	if (valuelen <= 0 || valuelen>LIBMEMCACHED_DEFAULT_VALUELEN)	return false;

	return true;
}
