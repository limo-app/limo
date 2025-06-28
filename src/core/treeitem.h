// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TREEITEM_H
#define TREEITEM_H

#include <vector>

//! [0]
template <typename T>
class TreeItem
{
public:
    explicit TreeItem(T *data, TreeItem *parent = nullptr);

    TreeItem *child(int number); int childCount() const; T *data() const;
    bool appendChild(TreeItem *child);
    bool appendChild(T *data);
    TreeItem *parent();
    bool removeChildren(int position, int count);
    int row() const;
    bool setData(T *value);

private:
    std::vector<TreeItem<T> *> m_childItems;
    T *itemData;
    TreeItem *m_parentItem;

    std::vector<T *> leaves;
    int rows = -1;
};
//! [0]

#include "treeitem.t.hpp"
#endif // TREEITEM_H
