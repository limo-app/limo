#include <QModelIndex>

#ifndef QMODELINDEXCHECK_H
#define QMODELINDEXCHECK_H

inline bool sameRow (const QModelIndex& a, const QModelIndex& b)
{
  return
    a.row() == b.row() &&
    a.parent() == b.parent() &&
    a.model() == b.model() &&
    a.internalPointer() == b.internalPointer();
}

template <typename T>
std::shared_ptr<T> qModelIndexToShared(const QModelIndex &index)
{
    auto rawPtr = index.internalPointer();
    if (!rawPtr) {
        return nullptr;
    }

    T *typedPtr = static_cast<T*>(rawPtr);
    return std::shared_ptr<T>(typedPtr, [](T*){});
}

#endif // QMODELINDEXCHECK_H
