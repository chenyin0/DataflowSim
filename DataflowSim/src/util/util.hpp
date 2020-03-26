#pragma once

/**
 * 使用说明
 *
 * Singleton为一个单例基类，任何需要的单例类只需继承自此类即为单例类
 * 被继承类需要将此类作为它的友元，构造函数写成私有
 * 通过getInstance()来获取实例的指针
 * 
 * Util为工具类，内部封装了常用的工具函数，全部为静态函数
 * 通过Util::xxx()来调用
 * 
 * Util::uRandom返回一个区间内的随机数，注意区间为左闭右闭区间
 * 
 * Util::splitString将一个字符串按给定的字符串进行切割
 * 例如splitString("aaababaabaaa", 'b')将返回{"aaa", "a", "aa", "aaa"}
 * 
 * Util::deleteAllMark将第一个字符串中的所有匹配第二个字符串的内容删除
 * 例如deleteAllMark("aaababaabaaa", 'b')会将原字符串更改为"aaaaaaaaa"
 * 
 * Util::findIndex返回数组中匹配值的下标，可以按元素查找，也可以自定义cmp匹配函数
 * 
 * Util::findKeyByValue根据值查找键，支持map/unordered_map
 * 不支持自定义cmp函数，需要的话再添加对应支持

 * Util::cast 对容器(vector/list/queue...)中元素进行类型转换，目前仅支持vector，其他容器自行添加
 * 
 * 行 2019.7.4
 *
 * 补充：
 * Singleton的子类保持单例仍然需要构造函数私有！
 *
 * 行 2019.10.6
 * 
 *  */

#include "../define/Define.hpp"

namespace DFSim
{
	template <class Typename>
	class Singleton
	{
	public:
		Singleton(const Singleton& b_) = delete;
		Singleton(Singleton&& b_) = delete;
		Singleton& operator=(const Singleton& b_) = delete;
		Singleton& operator=(Singleton&& b_) = delete;
	private:
		class Garbo final
		{
		public:
			Garbo() = default;
			Garbo(const Garbo& b_) = delete;
			Garbo(Garbo&& b_) = delete;
			Garbo& operator=(const Garbo& b_) = delete;
			Garbo& operator=(Garbo&& b_) = delete;

			~Garbo()
			{
				if (_instance)
				{
					delete _instance;
					_instance = nullptr;
				}
			}
		};

		static Typename* _instance;
		static Garbo _garbo;

	protected:
		Singleton() = default;
		~Singleton() = default;

	public:
		static Typename* getInstance()
		{
			if (_instance == nullptr)
				_instance = new Typename();
			return _instance;
		}
	};

	template <class Typename>
	Typename* Singleton<Typename>::_instance = nullptr;

	class Util final
	{
	public:

		/** generate a uniform random number in [a, b] */
		static uint uRandom(uint a_, uint b_)
		{
			const auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
			std::default_random_engine e(seed);
			const std::uniform_int_distribution<uint> u(a_, b_);
			return u(e);
		}

		/** calculate standard deviation */
		template<typename Type>
		static Type standardDeviation(const vector<Type> vec_)
		{
			Type sum = std::accumulate(vec_.begin(), vec_.end(), static_cast<Type>(0));
			Type mean = sum / vec_.size();
			Type accum = static_cast<Type>(0);
			std::for_each(vec_.begin(), vec_.end(), [&](Type num_)
			{
				accum += (num_ - mean) * (num_ - mean);
			});
			return std::sqrt(accum / vec_.size());
		}

		/** split string */
		static vector<string> splitString(const string& s_, const string& c_)
		{
			vector<string> v;
			auto pos2 = s_.find(c_);
			string::size_type pos1 = 0;

			while (string::npos != pos2)
			{
				v.push_back(s_.substr(pos1, pos2 - pos1));

				pos1 = pos2 + c_.size();
				pos2 = s_.find(c_, pos1);
			}

			if (pos1 != s_.length())
				v.push_back(s_.substr(pos1));

			return v;
		}

		static void deleteAllMark(string& s_, const string& mark_)
		{
			const auto n_size = mark_.size();

			while (true)
			{
				const auto pos = s_.find(mark_);
				if (pos == string::npos)
					return;

				s_.erase(pos, n_size);
			}
		}

		/** find element index in a vector */
		template <typename Type>
		static uint findIndex(const vector<Type>& vec_, Type& ele_)
		{
			auto itr = std::find(vec_.begin(), vec_.end(), ele_);
			return std::find(vec_.begin(), vec_.end(), ele_) == vec_.end() ? UINT_MAX : static_cast<uint>(std::distance(vec_.begin(), itr));
		}

		template <typename Type>
		static uint findIndex(const vector<Type>& vec_, Type&& ele_)
		{
			auto itr = std::find(vec_.begin(), vec_.end(), ele_);
			return itr == vec_.end() ? UINT_MAX : static_cast<uint>(std::distance(vec_.begin(), itr));
		}

		template <typename Type, typename Cmp>
		static uint findIndex(const vector<Type>& vec_, Cmp cmp_)
		{
			auto itr = std::find_if(vec_.begin(), vec_.end(), cmp_);
			return itr == vec_.end() ? UINT_MAX : static_cast<uint>(std::distance(vec_.begin(), itr));
		}

		template <typename Key, typename Value>
		static vector<uint> findKeyByValue(const unordered_map<Key, Value>& map_, const Value& v_)
		{
			vector<uint> result;
			for (auto& ele : map_)
				if (ele->second == v_)
					result.push_back(ele->first);
			return result;
		}

		template <typename Key, typename Value>
		static vector<uint> findKeyByValue(const map<Key, Value>& map_, const Value& v_)
		{
			vector<uint> result;
			for (auto& ele : map_)
				if (ele->second == v_)
					result.push_back(ele->first);
			return result;
		}

		template <typename Type, typename Cmp>
		static uint findMinIndex(vector<Type>& container_, Cmp cmp_)
		{
			if (container_.size() == 0)
				return UINT_MAX;

			auto min_itr = container_.begin();
			for (auto itr = container_.begin(); itr != container_.end(); ++itr)
			{
				if (cmp_(*itr, *min_itr))
					min_itr = itr;
			}

			return std::distance(container_.begin(), min_itr);
		}

		template <typename Type>
		static void eraseDuplicate(vector<Type>& vec_)
		{
			std::sort(std::begin(vec_), std::end(vec_));
			vec_.erase(std::unique(vec_.begin(), vec_.end()), vec_.end());
		}

		//template <typename Type>
		//static Type findSameValueInVectors(const vector<vector<Type>>& vec_)
		//{
		//	vector<Type> temp = vec_[0];
		//	for (uint i = 1; i < vec_.size(); i++)
		//	{
		//		uint index_vec = 0; uint index_tmp = 0;
		//		while (index_vec < vec_[i].size() && index_tmp < temp.size())
		//		{
		//			if (vec_[i][index_vec] == temp[index_tmp]) {
		//				++index_tmp;
		//				++index_vec;
		//			}
		//			else if (vec_[i][index_vec] < temp[index_tmp])
		//				index_vec++;
		//			else 
		//				temp.erase(temp.begin() + index_tmp);
		//		}	
		//	}
		//	if (temp.size())
		//		return temp.front();
		//	return UINT_MAX;
		//}

		template <typename Type>
		static Type findSameValueInVectors(const vector<vector<Type>>& vec_)
		{
			vector<Type> temp = vec_[0];
			auto curr = temp.begin();
			while (curr != temp.end()) {
				bool break_flag = false;
				for (uint j = 1; j < vec_.size(); j++) {
					if (std::find(vec_[j].begin(), vec_[j].end(), *curr) == vec_[j].end()) {
						curr=temp.erase(curr);
						break_flag = true;
						break;
					}
				}
				if(!break_flag){ ++curr; }
			}
			if (temp.size())
				return temp.front();
			return UINT_MAX;
		}

		static std::vector<std::string> splitString(std::string srcStr, std::string delimStr, bool repeatedCharIgnored)
		{
			std::vector<std::string> resultStringVector;
			std::replace_if(srcStr.begin(), srcStr.end(), [&](const char& c) {if (delimStr.find(c) != std::string::npos) { return true; } else { return false; }}/*pred*/, delimStr.at(0));//将出现的所有分隔符都替换成为一个相同的字符（分隔符字符串的第一个）
			size_t pos = srcStr.find(delimStr.at(0));
			std::string addedString = "";
			while (pos != std::string::npos) {
				addedString = srcStr.substr(0, pos);
				if (!addedString.empty() || !repeatedCharIgnored) {
					resultStringVector.push_back(addedString);
				}
				srcStr.erase(srcStr.begin(), srcStr.begin() + pos + 1);
				pos = srcStr.find(delimStr.at(0));
			}
			addedString = srcStr;
			if (!addedString.empty() || !repeatedCharIgnored) {
				resultStringVector.push_back(addedString);
			}
			return resultStringVector;
		}

		
		/** Convert element type in Vector */
		template<
			typename L, // 源类型
			typename R  // 目标类型
		>
			static std::vector<R>cast(const std::vector<L>& left, std::vector<R>*) 
		{
			std::vector<R> right;
			std::transform(
				left.begin(), // 输入容器迭代器(开始)
				left.end(), // 输入容器迭代器(结尾)
				std::insert_iterator<std::vector<R>>(right, right.begin()), // 输出迭代器
				[](L l)->R {return static_cast<R>(l);/*cast(l, (R*)nullptr);*/ } // lambda表达式调用上面的定义的Cast函数完成元素类型转换
				// 如果上面的cast函数无法满足要求，请自行根据需要扩展。
			);
			return std::move(right);
		}
	};
}