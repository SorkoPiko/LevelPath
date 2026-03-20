#include "SwitchNode.hpp"

#include "Geode/loader/Log.hpp"
#include "Geode/ui/Layout.hpp"

void SwitchNode::updateVisibility() const {
    for (size_t i = 0; i < nodes.size(); ++i) {
        nodes[i]->setVisible(i == currentIndex);
    }
}

SwitchNode* SwitchNode::create() {
    auto ret = new SwitchNode();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

int SwitchNode::addNode(CCNode* node) {
    if (nodes.empty()) setContentSize(node->getScaledContentSize());
    nodes.push_back(node);
    addChildAtPosition(node, Anchor::Center);
    updateVisibility();
    return static_cast<int>(nodes.size()) - 1;
}

void SwitchNode::setActiveIndex(const int index) {
    if (index < -1 || index >= nodes.size()) return;

    currentIndex = index;
    updateVisibility();
}

int SwitchNode::getActiveIndex() const {
    return currentIndex;
}