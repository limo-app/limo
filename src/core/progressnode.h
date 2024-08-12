/*!
 * \file progressnode.h
 * \brief Header for the ProgressNode class.
 */

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vector>


/*!
 * \brief Represents a node in a tree used to track the progress of a task.
 *
 * Each node in the tree represents the progress in a sub-task. Each sub-task has
 * a weight associated to it, which should be proportional to the time this task takes
 * to be completed.
 */
class ProgressNode
{
public:
  /*!
   * \brief Constructor.
   * \param id Id of this node. Used to index weights and children of parent.
   * \param weights If not empty: Weights of sub-tasks.
   * \param parent Parent of this node. If empty: This is a root node.
   */
  ProgressNode(int id, const std::vector<float>& weights, std::optional<ProgressNode*> parent);
  /*!
   * \brief Constructor for a root node.
   * \param progress_callback a callback function used by the root node to inform about
   * changes in the task progress.
   * \param weights If not empty: Weights of sub-tasks.
   */
  ProgressNode(std::function<void(float)> progress_callback,
               const std::vector<float>& weights = {});

  /*!
   * \brief Advances the current progress of this node by the given amount of steps.
   * This must be a leaf node.
   * \param num_steps Number steps to advance.
   */
  void advance(uint64_t num_steps = 1);
  /*!
   * \brief Returns the total number of steps in this task.
   * \return The number of steps.
   */
  int totalSteps() const;
  /*!
   * \brief Sets the total number of steps in this task.
   * \param total_steps The number of steps.
   */
  void setTotalSteps(uint64_t total_steps);
  /*!
   * \brief Returns the id of this node.
   * \return The id.
   */
  int id() const;
  /*!
   * \brief Adds new child nodes with given weights to this node.
   * \param weights The child weights.
   */
  void addChildren(const std::vector<float>& weights);
  /*!
   * \brief Returns a reference to the child with the given id.
   * \param id Target child id.
   * \return The child.
   */
  ProgressNode& child(int id);
  /*!
   * \brief Sets a callback function used by the root node to inform about changes in the
   * task progress.
   * \param set_progress The callback function.
   */
  void setProgressCallback(std::function<void(float)> progress_callback);
  /*!
   * \brief Returns the minimal progress interval after which the progress callback is called.
   * \return The interval.
   */
  float updateStepSize() const;
  /*!
   * \brief Sets the minimal progress interval after which the progress callback is called.
   * \param step_size The interval.
   */
  void setUpdateStepSize(float step_size);
  /*!
   * \brief Returns the current progress.
   * \return The progress.
   */
  float getProgress() const;

private:
  /*! \brief This nodes id. */
  int id_;
  /*! \brief Current step in this task. Only used for leaf nodes. */
  uint64_t cur_step_ = 0;
  /*! \brief Number of total steps in this task. Only used for leaf nodes. */
  uint64_t total_steps_;
  /*! \brief Current progress in this task. */
  float progress_ = 0.0f;
  /*! \brief Progress at the time of the last call to \ref set_progress_. */
  float prev_progress_ = 0.0f;
  /*! \brief minimal progress interval after which \ref set_progress_ is called. */
  float update_step_size_ = 0.01f;
  /*! \brief The parent of this, if this is not the root. */
  std::optional<ProgressNode*> parent_;
  /*! \brief Weights of children. */
  std::vector<float> weights_;
  /*! \brief Children representing sub-tasks of this task. */
  std::vector<ProgressNode> children_;

  /*!
   * \brief Callback function used by the root node to inform about changes in the
   * task progress.
   */
  std::function<void(float)> set_progress_ = [](float f) {};
  /*!
   * \brief Sets the current progress of this node to the weighted sum of the current
   * progresses of its children.
   */
  void updateProgress();
  /*!
   * \brief Informs this nodes parent of a change in progress.
   *
   * If this is a root node and the change of progress since the last update exceeds
   * \ref update_step_size_ : Call \ref set_progress_.
   */
  void propagateProgress();
};
