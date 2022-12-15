#pragma once

/**
 * ʹ��˵��
 *
 * SingletonΪһ���������࣬�κ���Ҫ�ĵ�����ֻ��̳��Դ��༴Ϊ������
 * ���̳�����Ҫ��������Ϊ������Ԫ�����캯��д��˽��
 * ͨ��getInstance()����ȡʵ����ָ��
 * 
 * UtilΪ�����࣬�ڲ���װ�˳��õĹ��ߺ�����ȫ��Ϊ��̬����
 * ͨ��Util::xxx()������
 * 
 * Util::uRandom����һ�������ڵ��������ע������Ϊ����ұ�����
 * 
 * Util::splitString��һ���ַ������������ַ��������и�
 * ����splitString("aaababaabaaa", 'b')������{"aaa", "a", "aa", "aaa"}
 * 
 * Util::deleteAllMark����һ���ַ����е�����ƥ��ڶ����ַ���������ɾ��
 * ����deleteAllMark("aaababaabaaa", 'b')�Ὣԭ�ַ�������Ϊ"aaaaaaaaa"
 * 
 * Util::findIndex����������ƥ��ֵ���±꣬���԰�Ԫ�ز��ң�Ҳ�����Զ���cmpƥ�亯��
 * 
 * Util::findKeyByValue����ֵ���Ҽ���֧��map/unordered_map
 * ��֧���Զ���cmp��������Ҫ�Ļ������Ӷ�Ӧ֧��

 * Util::cast ������(vector/list/queue...)��Ԫ�ؽ�������ת����Ŀǰ��֧��vector������������������
 * 
 * �� 2019.7.4
 *
 * ���䣺
 * Singleton�����ౣ�ֵ�����Ȼ��Ҫ���캯��˽�У�
 *
 * �� 2019.10.6
 * 
 *  */

#include "../define/Define.hpp"
#include "limits.h"
#include <random>

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
        static uint64_t uRandom(uint64_t a_, uint64_t b_)
        {
            const auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            std::default_random_engine e(seed);
            std::uniform_int_distribution<uint64_t> u(a_, b_);
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
        static uint64_t findIndex(const vector<Type>& vec_, Type& ele_)
        {
            auto itr = std::find(vec_.begin(), vec_.end(), ele_);
            return std::find(vec_.begin(), vec_.end(), ele_) == vec_.end() ? UINT_MAX : static_cast<uint64_t>(std::distance(vec_.begin(), itr));
        }

        template <typename Type>
        static uint64_t findIndex(const vector<Type>& vec_, Type&& ele_)
        {
            auto itr = std::find(vec_.begin(), vec_.end(), ele_);
            return itr == vec_.end() ? UINT_MAX : static_cast<uint64_t>(std::distance(vec_.begin(), itr));
        }

        template <typename Type, typename Cmp>
        static uint64_t findIndex(const vector<Type>& vec_, Cmp cmp_)
        {
            auto itr = std::find_if(vec_.begin(), vec_.end(), cmp_);
            return itr == vec_.end() ? UINT_MAX : static_cast<uint64_t>(std::distance(vec_.begin(), itr));
        }

        template <typename Key, typename Value>
        static vector<uint64_t> findKeyByValue(const unordered_map<Key, Value>& map_, const Value& v_)
        {
            vector<uint64_t> result;
            for (auto& ele : map_)
                if (ele->second == v_)
                    result.push_back(ele->first);
            return result;
        }

        template <typename Key, typename Value>
        static vector<uint64_t> findKeyByValue(const map<Key, Value>& map_, const Value& v_)
        {
            vector<uint64_t> result;
            for (auto& ele : map_)
                if (ele->second == v_)
                    result.push_back(ele->first);
            return result;
        }

        template <typename Type, typename Cmp>
        static uint64_t findMinIndex(vector<Type>& container_, Cmp cmp_)
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
        //    vector<Type> temp = vec_[0];
        //    for (uint64_t i = 1; i < vec_.size(); i++)
        //    {
        //        uint64_t index_vec = 0; uint64_t index_tmp = 0;
        //        while (index_vec < vec_[i].size() && index_tmp < temp.size())
        //        {
        //            if (vec_[i][index_vec] == temp[index_tmp]) {
        //                ++index_tmp;
        //                ++index_vec;
        //            }
        //            else if (vec_[i][index_vec] < temp[index_tmp])
        //                index_vec++;
        //            else 
        //                temp.erase(temp.begin() + index_tmp);
        //        }    
        //    }
        //    if (temp.size())
        //        return temp.front();
        //    return UINT_MAX;
        //}

        template <typename Type>
        static Type findSameValueInVectors(const vector<vector<Type>>& vec_)
        {
            vector<Type> temp = vec_[0];
            auto curr = temp.begin();
            while (curr != temp.end()) {
                bool break_flag = false;
                for (uint64_t j = 1; j < vec_.size(); j++) {
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
            std::replace_if(srcStr.begin(), srcStr.end(), [&](const char& c) {if (delimStr.find(c) != std::string::npos) { return true; } else { return false; }}/*pred*/, delimStr.at(0));//�����ֵ����зָ������滻��Ϊһ����ͬ���ַ����ָ����ַ����ĵ�һ����
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

        template <typename Type>
        static vector<Type> removeDuplicatesKeepSequence(const vector<Type>& vec_)
        {
            vector<Type> vecTmp;
            for (auto& i : vec_)
            {
                if (find(vecTmp.begin(), vecTmp.end(), i) == vecTmp.end())
                {
                    vecTmp.push_back(i);
                }
            }

            return vecTmp;
        }

        
        /** Convert element type in Vector */
        template<
            typename L, // Դ����
            typename R  // Ŀ������
        >
            static std::vector<R>cast(const std::vector<L>& left, std::vector<R>*) 
        {
            std::vector<R> right;
            std::transform(
                left.begin(), // ��������������(��ʼ)
                left.end(), // ��������������(��β)
                std::insert_iterator<std::vector<R>>(right, right.begin()), // ���������
                [](L l)->R { return static_cast<R>(l);/*cast(l, (R*)nullptr);*/ } // lambda����ʽ��������Ķ����Cast�������Ԫ������ת��
                // ��������cast�����޷�����Ҫ�������и�����Ҫ��չ��
            );
            return std::move(right);
        }
    };
}