// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TREEITEM_H
#define TREEITEM_H

#include <vector>
#include <memory>

//! [0]
template <typename T>
class TreeItem
{
public:
    explicit TreeItem(std::vector<T> data, TreeItem *parent = nullptr);

    TreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    T data(int column) const;
    bool insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);
    bool appendChild(std::shared_ptr<TreeItem> child);
    bool appendChild(std::vector<T> data);
    std::shared_ptr<TreeItem> parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int row() const;
    bool setData(int column, const T &value);

private:
    std::vector<std::shared_ptr<TreeItem>> m_childItems;
    std::vector<T> itemData;
    std::shared_ptr<TreeItem> m_parentItem;
};
//! [0]

#include "treeitem.t.hpp"
#endif // TREEITEM_H
