// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

/*
    treeitem.cpp

    A container for items of data supplied by the simple tree model.
*/

#ifndef TREEITEM_T_H
#define TREEITEM_T_H

#include "deployerentry.hpp"
#include "treeitem.h"
#include <utility>

template <typename T>
TreeItem<T>::TreeItem(std::shared_ptr<T> data, std::shared_ptr<TreeItem<T>> parent)
    : itemData(std::move(data)), m_parentItem(parent)
{
  m_childItems = std::vector<std::shared_ptr<TreeItem>>();
}

template <typename T>
std::shared_ptr<TreeItem<T>> TreeItem<T>::child(int row)
{
  return this->operator[](row);
}

template <typename T>
int TreeItem<T>::childCount() const
{
  return int(m_childItems.size());
}

template <typename T>
int TreeItem<T>::row() const
{
  if (!m_parentItem)
    return 0;
  const auto it = std::find_if(m_parentItem->m_childItems.cbegin(), m_parentItem->m_childItems.cend(),
                                       [this](const auto& treeItem) {
                                       return treeItem.get() == this;
                                       });

  if (it != m_parentItem->m_childItems.cend())
    return std::distance(m_parentItem->m_childItems.cbegin(), it);
  return -1;
}

template <typename T>
std::shared_ptr<TreeItem<T>> TreeItem<T>::parent()
{
    return m_parentItem;
}

template <typename T>
bool TreeItem<T>::setData(std::unique_ptr<T> value)
{
  itemData = std::move(value);
  markDirty();
  return true;
}

template <typename T>
std::vector<std::weak_ptr<TreeItem<T>>> TreeItem<T>::preOrderTraversal()
{
  if (!dirty) {
    return traversal_cache;
  }

  traversal_cache.clear();
  traversalItems_cache.clear();

  std::vector<std::weak_ptr<TreeItem<T>>> result = std::vector<std::weak_ptr<TreeItem<T>>>();
  if (itemData)
    result.emplace_back(this->weak_from_this());
  for (const auto &child : m_childItems)
  {
    auto child_result = child->preOrderTraversal();
    result.insert(result.end(), child_result.begin(), child_result.end());
  }

  traversal_cache = result;
  for (auto item : traversal_cache)
    if (auto lockedItem = item.lock())
      traversalItems_cache.push_back(lockedItem->getData());

  dirty = false;
  return result;
}

template <typename T>
void TreeItem<T>::emplace_back(std::shared_ptr<TreeItem<T>> data)
{
  m_childItems.emplace_back(std::move(data));
  markDirty();
}

template <typename T>
void TreeItem<T>::emplace_back(std::shared_ptr<T> data)
{
  m_childItems.emplace_back(make_shared<TreeItem<T>>(std::move(data), this->shared_from_this()));
  markDirty();
}

template <typename T>
Json::Value TreeItem<T>::toJson() const {
  Json::Value json;
  if (itemData) {
    json = itemData->toJson();
  }
  for (const auto &child : m_childItems) {
    json["children"].append(child->toJson());
  }
  return json;
}

template <typename T>
void TreeItem<T>::swapChild(int from, int to) {
  if (from < 0 || to < 0 || from >= m_childItems.size() || to >= m_childItems.size() || from == to) {
    throw std::out_of_range("Invalid indices for swapChild");
  }
  iter_swap(m_childItems.begin() + from, m_childItems.begin() + to);
  markDirty();
}

template <typename T>
void TreeItem<T>::swapNodes(std::shared_ptr<TreeItem<T>> nodeA, std::shared_ptr<TreeItem<T>> nodeB) {
  if (nodeA->isAncestor(nodeB) || nodeB->isAncestor(nodeA))
    throw std::logic_error("Cannot swap nodes that are ancestors of each other");
  std::swap(nodeA, nodeB);
  markDirty();
}

template <typename T>
bool TreeItem<T>::isAncestor(std::shared_ptr<TreeItem<T>> potentialAncestor) {
  if (this->shared_from_this() == potentialAncestor) return true;
  else if (this->parent() != nullptr)
    return this->parent()->isAncestor(potentialAncestor);
  return false;
}

template <typename T>
void TreeItem<T>::insert(int position, std::shared_ptr<TreeItem> item) {
  m_childItems.insert(m_childItems.begin() + position, std::move(item));
  markDirty();
}

template <typename T>
void TreeItem<T>::remove(std::weak_ptr<TreeItem<T>> child) {
  auto childPtr = child.lock();
  if (!childPtr) {
    auto it = std::find_if(m_childItems.begin(), m_childItems.end(),
                          [](const auto& ptr) { return ptr.get() == nullptr; });
    if (it != m_childItems.end()) {
      m_childItems.erase(it);
    }
    markDirty();
    return;
  }

  auto it = std::find_if(m_childItems.begin(), m_childItems.end(),
                         [childPtr](const auto& ptr) { return ptr == childPtr; });
  if (it != m_childItems.end()) {
    // 2. Extract the doomed child
    auto doomedChild = std::move(*it);

    // 3. Steal its children (they're now orphaned)
    auto orphans = std::move(doomedChild->m_childItems);

    // 4. Remove the child (will be destroyed here)
    m_childItems.erase(it);

    // 5. Adopt orphans at same position
    m_childItems.insert(it,
                        std::make_move_iterator(orphans.begin()),
                        std::make_move_iterator(orphans.end()));

    // 6. Update parent pointers
    for (auto& adopted : orphans) {
      adopted->setParent(this->shared_from_this());
    }
    markDirty();
  }
}

template <typename T>
std::shared_ptr<TreeItem<T>> TreeItem<T>::markNull(std::shared_ptr<TreeItem<T>> item) {
  if (dirty) refresh();
  auto it = std::find_if(m_childItems.begin(), m_childItems.end(),
                         [item](const auto& ptr) { return ptr.get() == item.get(); });
  if (it != m_childItems.end()) {
    markDirty();
    return std::move(*it);
  } else {
    return nullptr;
  }
}

template <typename T>
void TreeItem<T>::setParent(std::shared_ptr<TreeItem<T>> parent) {
  m_parentItem = parent;
  markDirty();
}

template <typename T>
void TreeItem<T>::erase(std::weak_ptr<T> item) {
  if (dirty) refresh();
  auto found = std::ranges::find_if(
    traversal_cache.begin(),
    traversal_cache.end(),
    [item](const std::weak_ptr<TreeItem<T>> e) { return e.lock()->getData() == item.lock(); });
  (*found).lock()->parent()->remove(*found);
}

template <typename T>
void TreeItem<T>::refresh() {
  preOrderTraversal();
}

template <typename T>
void TreeItem<T>::markDirty() {
  dirty = true;
  if (parent() != nullptr) {
    parent()->markDirty();
  }
}

template class TreeItem<DeployerEntry>;

#endif
