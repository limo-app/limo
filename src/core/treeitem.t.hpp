// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

/*
    treeitem.cpp

    A container for items of data supplied by the simple tree model.
*/

#include "core/deployerentry.hpp"
#include "treeitem.h"
#include <utility>

//! [0]
template <typename T>
TreeItem<T>::TreeItem(T *data, TreeItem *parent)
    : itemData(std::move(data)), m_parentItem(parent)
{
  m_childItems = std::vector<TreeItem *>();
}
//! [0]

//! [1]
template <typename T>
TreeItem<T> *TreeItem<T>::child(int number)
{
    return (number >= 0 && number < childCount())
        ? m_childItems.at(number) : nullptr;
}
//! [1]

//! [2]
template <typename T>
int TreeItem<T>::childCount() const
{
    return int(m_childItems.size());
}
//! [2]

//! [3]
template <typename T>
int TreeItem<T>::row() const
{
    if (!m_parentItem)
        return 0;
    const auto it = std::ranges::find_if(m_parentItem->m_childItems.cbegin(), m_parentItem->m_childItems.cend(),
                                 [this](const TreeItem *treeItem) {
        return treeItem == this;
    });

    if (it != m_parentItem->m_childItems.cend())
        return std::distance(m_parentItem->m_childItems.cbegin(), it);
    return -1;
}
//! [3]

//! [5]
template <typename T>
T *TreeItem<T>::data() const
{
    return itemData;
}
//! [5]

//! [6]
// template <typename T>
// bool TreeItem<T>::insertChildren(int position, int count, int columns)
// {
//     if (position < 0 || position > m_childItems.size())
//         return false;
//
//     for (int row = 0; row < count; ++row) {
//         std::vector<T> data(columns);
//         m_childItems.insert(m_childItems.cbegin() + position,
//                 std::make_unique<TreeItem>(data, this));
//     }
//
//     return true;
// }
//! [6]

template <typename T>
bool TreeItem<T>::appendChild(TreeItem *child)
{
  m_childItems.push_back(child);
  return true;
}

template <typename T>
bool TreeItem<T>::appendChild(T *data)
{
  m_childItems.push_back(new TreeItem<T>(data, this));
  return true;
}

//! [8]
template <typename T>
TreeItem<T> *TreeItem<T>::parent()
{
    return m_parentItem;
}
//! [8]

//! [9]
template <typename T>
bool TreeItem<T>::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        m_childItems.erase(m_childItems.cbegin() + position);

    return true;
}
//! [9]

//! [10]
template <typename T>
bool TreeItem<T>::setData(T *value)
{
    itemData = value;
    return true;
}
//! [10]

template class TreeItem<DeployerEntry>;
