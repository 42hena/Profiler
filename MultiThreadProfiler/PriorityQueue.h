#pragma once

#include <functional>

template<typename T, typename Compare = std::less<T>>
class PriorityQueue
{
public:
    PriorityQueue();
    PriorityQueue(int maxCount);
    ~PriorityQueue();

public:
    PriorityQueue(const PriorityQueue&) = delete;
    PriorityQueue& operator=(const PriorityQueue&) = delete;
    PriorityQueue(PriorityQueue&&) = delete;
    PriorityQueue& operator=(PriorityQueue&&) = delete;

public:
    bool Empty() const;
    size_t Size() const;
    bool Top(T&) const;

    bool Push(const T& value);
    bool Pop(T& value);

private:
    void HeapifyUp(int index);
    void HeapifyDown(int index);

private:
    T* _data;
    Compare _comp;
    int _maxCount;
    int _index;
};

template<typename T, typename Compare>
inline PriorityQueue<T, Compare>::PriorityQueue()
    : _maxCount(32)
    , _index(0)
{
    _data = new T[_maxCount];
}

template<typename T, typename Compare>
inline PriorityQueue<T, Compare>::PriorityQueue(int maxCount)
    : _maxCount(maxCount)
    , _index(0)
{
    _data = new T[_maxCount];
}

template<typename T, typename Compare>
inline PriorityQueue<T, Compare>::~PriorityQueue()
{
    delete[] _data;
}

template<typename T, typename Compare>
inline bool PriorityQueue<T, Compare>::Empty() const
{
    if (_index != 0)
    {
        return false;
    }
    return true;
}

template<typename T, typename Compare>
inline size_t PriorityQueue<T, Compare>::Size() const
{
    return _index;
}

template<typename T, typename Compare>
inline bool PriorityQueue<T, Compare>::Top(T& value) const
{
    if (Empty())
    {
        return false;
    }
    value = _data[0];
    return true;
}

template<typename T, typename Compare>
inline bool PriorityQueue<T, Compare>::Push(const T& value)
{
    if (_index == _maxCount)
    {
        return false;
    }

    _data[_index] = value;
    _index += 1;

    HeapifyUp(_index - 1);
    return true;
}

template<typename T, typename Compare>
inline bool PriorityQueue<T, Compare>::Pop(T& value)
{
    if (Top(value) == false)
    {
        return false;
    }

    std::swap(_data[0], _data[_index - 1]);
    _index -= 1;

    if (Empty() == false)
    {
        HeapifyDown(0);
    }

    return true;
}

template<typename T, typename Compare>
inline void PriorityQueue<T, Compare>::HeapifyUp(int index)
{
    while (index > 0)
    {
        int parentIndex = (index - 1) / 2;

        // Compare ±‚¡ÿ
        if (!_comp(_data[index], _data[parentIndex]))
        {
            break;
        }

        std::swap(_data[index], _data[parentIndex]);
        index = parentIndex;
    }
}

template<typename T, typename Compare>
inline void PriorityQueue<T, Compare>::HeapifyDown(int index)
{
    int size = _index;

    while (true)
    {
        int leftIndex = index * 2 + 1;
        int rightIndex = index * 2 + 2;
        int bestIndex = index;

        if (leftIndex < size && _comp(_data[leftIndex], _data[bestIndex]))
        {
            bestIndex = leftIndex;
        }

        if (rightIndex < size && _comp(_data[rightIndex], _data[bestIndex]))
        {
            bestIndex = rightIndex;
        }

        if (bestIndex == index)
        {
            break;
        }

        std::swap(_data[index], _data[bestIndex]);
        index = bestIndex;
    }
}
