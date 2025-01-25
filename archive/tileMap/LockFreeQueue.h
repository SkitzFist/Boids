#ifndef BOIDS_LOCK_FREE_QUEUE_H
#define BOIDS_LOCK_FREE_QUEUE_H

#include <atomic>

template <typename T>
class LockFreeQueue {
  public:
    LockFreeQueue() {
        Node* dummy = new Node();
        head.store(dummy);
        tail.store(dummy);
    }

    ~LockFreeQueue() {
        while (Node* node = head.load()) {
            head.store(node->next);
            delete node;
        }
    }

    void enqueue(const T& value) {
        Node* newNode = new Node(value);
        Node* prevHead = head.exchange(newNode, std::memory_order_acq_rel);
        prevHead->next.store(newNode, std::memory_order_release);
    }

    bool dequeue(T& result) {
        Node* tailNode = tail.load(std::memory_order_acquire);
        Node* nextNode = tailNode->next.load(std::memory_order_acquire);

        if (nextNode) {
            result = nextNode->value;
            tail.store(nextNode, std::memory_order_release);
            delete tailNode;
            return true;
        }
        return false;
    }

  private:
    struct Node {
        T value;
        std::atomic<Node*> next;
        Node() : next(nullptr) {}
        Node(const T& val) : value(val), next(nullptr) {}
    };

    std::atomic<Node*> head;
    std::atomic<Node*> tail;
};

#endif
