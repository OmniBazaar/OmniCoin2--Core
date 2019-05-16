#pragma once

#include <fc/thread/mutex.hpp>
#include <fc/thread/scoped_lock.hpp>

namespace graphene
{
    namespace net
    {
        namespace detail
        {
            /*******
             * A class to wrap std::unordered_set for multithreading
             */
            template <class Key, class Hash = std::hash<Key>, class Pred = std::equal_to<Key> >
            class concurrent_unordered_set : private std::unordered_set<Key, Hash, Pred>
            {
            private:
                mutable fc::mutex mux;

            public:
                // iterations require a lock. This exposes the mutex. Use with care (i.e. lock_guard)
                fc::mutex& get_mutex()const { return mux; }

                // insertion
                std::pair< typename std::unordered_set<Key, Hash, Pred>::iterator, bool> emplace( Key key)
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::emplace( key );
                }
                std::pair< typename std::unordered_set<Key, Hash, Pred>::iterator, bool> insert (const Key& val)
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::insert( val );
                }
                // size
                size_t size() const
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::size();
                }
                bool empty() const noexcept
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::empty();
                }
                // removal
                void clear() noexcept
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    std::unordered_set<Key, Hash, Pred>::clear();
                }
                typename std::unordered_set<Key, Hash, Pred>::iterator erase(
                        typename std::unordered_set<Key, Hash, Pred>::const_iterator itr)
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::erase( itr);
                }
                size_t erase( const Key& key)
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::erase( key );
                }
                // iteration
                typename std::unordered_set<Key, Hash, Pred>::iterator begin() noexcept
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::begin();
                }
                typename std::unordered_set<Key, Hash, Pred>::const_iterator begin() const noexcept
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::begin();
                }
                typename std::unordered_set<Key, Hash, Pred>::local_iterator begin(size_t n)
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::begin(n);
                }
                typename std::unordered_set<Key, Hash, Pred>::const_local_iterator begin(size_t n) const
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::begin(n);
                }
                typename std::unordered_set<Key, Hash, Pred>::iterator end() noexcept
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::end();
                }
                typename std::unordered_set<Key, Hash, Pred>::const_iterator end() const noexcept
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::end();
                }
                typename std::unordered_set<Key, Hash, Pred>::local_iterator end(size_t n)
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::end(n);
                }
                typename std::unordered_set<Key, Hash, Pred>::const_local_iterator end(size_t n) const
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::end(n);
                }
                // search
                typename std::unordered_set<Key, Hash, Pred>::const_iterator find(Key key)
                {
                    fc::scoped_lock<fc::mutex> lock(mux);
                    return std::unordered_set<Key, Hash, Pred>::find(key);
                }
            };
        }
    }
}
