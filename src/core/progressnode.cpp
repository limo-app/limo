#include "progressnode.h"
#include <limits>
#include <numeric>


ProgressNode::ProgressNode(int id,
                           const std::vector<float>& weights,
                           std::optional<ProgressNode*> parent) : id_(id), parent_(parent)
{
  addChildren(weights);
}

ProgressNode::ProgressNode(std::function<void(float)> progress_callback,
                           const std::vector<float>& weights)
{
  addChildren(weights);
  setProgressCallback(progress_callback);
}

void ProgressNode::advance(uint64_t num_steps)
{
  if(!children_.empty())
    throw std::runtime_error("Cannot advance progress for a node with children.");
  cur_step_ += num_steps;
  if(total_steps_ == 0)
    progress_ = 1.0f;
  else
    progress_ = std::min(static_cast<float>(cur_step_) / total_steps_, 1.0f);
  propagateProgress();
}

int ProgressNode::totalSteps() const
{
  return total_steps_;
}

void ProgressNode::setTotalSteps(uint64_t total_steps)
{
  if(!children_.empty())
    throw std::runtime_error("Cannot set total steps for a node with children.");
  total_steps_ = total_steps;
}

int ProgressNode::id() const
{
  return id_;
}

void ProgressNode::addChildren(const std::vector<float>& weights)
{
  weights_ = weights;
  for(float& weight : weights_)
    weight = std::abs(weight);
  float sum = std::accumulate(weights_.begin(), weights_.end(), 0.0f);
  if(sum == 0.0f)
    sum = 1.0f;
  for(float& weight : weights_)
    weight /= sum;
  for(int i = 0; i < weights_.size(); i++)
    children_.push_back({ i, {}, this });
}

ProgressNode& ProgressNode::child(int id)
{
  return children_[id];
}

void ProgressNode::setProgressCallback(std::function<void(float)> progress_callback)
{
  set_progress_ = progress_callback;
  set_progress_(progress_);
}

float ProgressNode::updateStepSize() const
{
  return update_step_size_;
}

void ProgressNode::setUpdateStepSize(float step_size)
{
  update_step_size_ = step_size;
}

float ProgressNode::getProgress() const
{
  return progress_;
}

void ProgressNode::updateProgress()
{
  progress_ = 0.0f;
  for(int i = 0; i < weights_.size(); i++)
    progress_ += weights_[i] * children_[i].progress_;
  propagateProgress();
}

void ProgressNode::propagateProgress()
{
  if(parent_)
    (*parent_)->updateProgress();
  else if(progress_ - prev_progress_ > update_step_size_ ||
          std::abs(1.0f - progress_) <= std::numeric_limits<float>::epsilon() &&
            std::abs(1.0f - prev_progress_) > std::numeric_limits<float>::epsilon())
  {
    set_progress_(progress_);
    prev_progress_ = progress_;
  }
}
