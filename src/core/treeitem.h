// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TREEITEM_H
#define TREEITEM_H

#include <json/value.h>
#include <vector>

template <typename T>
class TreeItem : public std::enable_shared_from_this<TreeItem<T>>
{
public:
  TreeItem(std::shared_ptr<T> data, std::shared_ptr<TreeItem<T>> parent = nullptr);

  int childCount() const;
  std::shared_ptr<TreeItem<T>> parent();
  void setParent(std::shared_ptr<TreeItem<T>> parent);
  int row() const;
  bool setData(std::unique_ptr<T> value);
  std::shared_ptr<T> getData() const { return itemData; }
  std::vector<std::weak_ptr<TreeItem<T>>> getTraversal() {
    if (dirty) refresh();
    return traversal_cache;
  }
  std::vector<std::weak_ptr<T>> getTraversalItems() {
    if (dirty) refresh();
    return traversalItems_cache;
  }

  void emplace_back(std::shared_ptr<TreeItem<T>> data);
  void emplace_back(std::shared_ptr<T> data);
  void insert(int position, std::shared_ptr<TreeItem> item);
  void remove(std::weak_ptr<TreeItem<T>> item);
  void erase(std::weak_ptr<T> item);
  std::shared_ptr<TreeItem<T>> markNull(std::shared_ptr<TreeItem<T>> item);
  void markDirty();

  int size() const {
    return m_childItems.size();
  }
  bool empty() const { return m_childItems.empty(); }
  std::shared_ptr<TreeItem<T>> operator[](int i) {
    return m_childItems[i];
  }
  std::shared_ptr<TreeItem<T>> child(int number);
  std::vector<std::weak_ptr<T>>::iterator begin() {
    if (dirty) refresh();
    return traversalItems_cache.begin();
  }
  std::vector<std::weak_ptr<T>>::iterator end() {
    if (dirty) refresh();
    return traversalItems_cache.end();
  }
  std::shared_ptr<TreeItem<T>> back() {
    static std::shared_ptr<TreeItem<T>> emptyPtr;
    if (m_childItems.empty()) return emptyPtr;
    return m_childItems.back();
  }

  void swapChild(int from, int to);
  void swapNodes(std::shared_ptr<TreeItem<T>> nodeA, std::shared_ptr<TreeItem<T>> nodeB);
  void refresh();
  Json::Value toJson() const;

private:
  std::vector<std::shared_ptr<TreeItem<T>>> m_childItems;
  std::shared_ptr<T> itemData;
  std::shared_ptr<TreeItem<T>> m_parentItem;

  bool dirty = true;
  std::vector<std::weak_ptr<TreeItem<T>>> traversal_cache;
  std::vector<std::weak_ptr<T>> traversalItems_cache;
  int rows = -1;

  std::vector<std::weak_ptr<TreeItem<T>>> preOrderTraversal();
  bool isAncestor(std::shared_ptr<TreeItem<T>> potentialAncestor);
};

#include "treeitem.t.hpp"
#endif // TREEITEM_H
