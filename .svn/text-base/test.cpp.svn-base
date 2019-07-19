#include <iostream>
#include "./CHredis/CHiredis.h"

using namespace std;

struct MyStruct
{
	int numid;
	string name;
	int year;

	MyStruct() { reset(); }

	void reset()
	{
		numid = 0;
		name.clear();
		year = 0;
	}

	friend bostream& operator<<(bostream& bos, const MyStruct& s)
	{
		bos << s.numid;
		bos << s.name;
		bos << s.year;
		return bos;
	}

	friend bistream& operator >> (bistream& bis, MyStruct& s)
	{
		s.reset();
		bis >> s.numid;
		bis >> s.name;
		bis >> s.year;
		return bis;
	}
};


int main()
{
	vector<string> veclist;
	std::vector<std::pair<std::string, long long> >vecl;
	string str;
	CCHiredis redis;
	redis.HiConnect("127.0.0.1", 6379);
	long long num;
	MyStruct me, tmp;
	me.name = "sunzhen";
	me.numid = 1;
	me.year = 27;

	tmp.name = "zhuhua";
	tmp.numid = 2;
	tmp.year = 4;

	map<string, MyStruct> vectmp;

	int tmpint;

	//redis.HiRedisCommand("ping");

	//redis.Replace_String("name", "sunzhen", 0);

	//redis.SetHashKeyFiled("website", "google", "www.google.com");
	//redis.SetHashKeyFiled("website", "baidu", "www.baidu.com");
	//redis.SetHashKeyFiled("website", "taobao", "www.taobao.com");
	//redis.SetHashKeyFiled("website", "zhihu", "www.zhihu.com");

	

	redis.Set_String("key", "sz123");
	redis.Set_KeyExpiration("key", 60);

// 	redis.Set_HashData("website", "mymsg1", 1000);
// 	redis.Set_HashData("website", "mymsg2", tmp);
// 	tmpint = redis.HashKeyFieldAddNum("website", "mymsg1", 1000);
// 	redis.Get_HashData("website", "mymsg1", str);
// 	redis.Del("website");

	redis.Set_ZsetMultiData("rate", 4, 100000, "sunzhen", 110, "zhuhua");
	num = redis.Get_ZsetMemberScore("rate", "sunzhen");
	num = redis.Set_ZsetMemberAddNum("rate", 900, "sunzhen");
	tmpint = redis.Get_ZsetRangeByIndex("rate", 0, -1, vecl, DESC);
	/*redis.Get_SSetRangeByScore("rate", 0, 10000, vecl, false, true, true);
	tmpint = redis.Get_SSetMemberRank("rate", "zhuhua", DESC);*/

	//tmpint = redis.Del_SSetMember("rate", 2, "zhangsan", "lisi");
	//tmpint = redis.Exist_Key("money");

	return 0;
}