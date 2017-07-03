#ifndef __COMMON_SHARDING_H
#define __COMMON_SHARDING_H

#include <map>
#include "hashkit.h"

struct MurmurHash 
{
	static uint32_t hash(const std::string& key)
	{
		return hash_murmur(key.data(), key.size());
	}
};

template <typename Shard, typename Hash=MurmurHash>
class Sharding
{
public:
	typedef std::map<int, Shard*> Shards;

	Sharding(int shardnum = 1) : shardnum_(shardnum) 
	{
	}

	~Sharding()
	{
	}

	void setShardNum(int shardnum)
	{
		shardnum_ = shardnum;
	}

	int shardNum() { return shardnum_; }

	bool isReady()
	{
		for (int i = 0; i < shardnum_; i++)
			if (shards_.find(i) == shards_.end())
				return false;
		return true;
	}

	bool addShard(Shard* shard)
	{
		if (!shard) return false;

		if (shard->shard() == -1) return false;

		if (shards_.find(shard->shard()) != shards_.end())
			return false;

		shards_[shard->shard()] = shard;
		return true;
	}

	//
	// MAXINT divided by SHARDNUM
	//
	// N: total sharding number(2^x)
	// M: 2^32
	// 
	// shard0  shard1   shard2       shard(N-1)
	// |________|________|............|
	// 0      M/N       2M/N          M
	//
	Shard* getShardByHash(uint32_t hashcode)
	{
		return shardnum_ > 0 ? getShardByID(hashcode / (4294967296UL/shardnum_)) : NULL;
	}

	Shard* getShardByKey(const std::string& key)
	{
		return getShardByHash(Hash::hash(key));
	}

	Shard* getShardByID(int shard)
	{
		typename Shards::iterator it = shards_.find(shard);
		if (it != shards_.end())
			return it->second;
		return NULL;
	}

protected:
	Shards shards_;

	// total shards number(best to be 2^n)
	int shardnum_;
};

#endif // __COMMON_SHARDING_H