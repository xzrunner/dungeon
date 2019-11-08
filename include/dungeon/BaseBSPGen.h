#pragma once

#include <SM_Vector.h>
#include <SM_Rect.h>

#include <memory>
#include <vector>

namespace dungeon
{

class BaseBSPGen
{
public:
    BaseBSPGen(const sm::vec2& size, const sm::vec2& split_sz,
        const sm::vec2& min_sz, uint32_t seed = 0);

    std::vector<sm::rect> GetAllRooms() const;

private:
    void BuildBSP(const sm::vec2& size,
        const sm::vec2& split_sz, const sm::vec2& min_sz);
    void BuildDungeon(const sm::vec2& min_sz);

private:
    enum class SplitType
    {
        None,
        Hori,
        Vert,
    };

    class Node
    {
    public:
        Node(const sm::rect& r);

        bool Split(const sm::vec2& split_sz,
            const sm::vec2& min_sz);

    private:
        std::unique_ptr<Node> m_kids[2];

        sm::rect m_rect;
        sm::rect m_room;

        SplitType m_split = SplitType::None;

        friend class BaseBSPGen;

    }; // Node

private:
    std::unique_ptr<Node> m_root = nullptr;

    uint32_t m_seed = 0;

}; // BaseBSPGen

}