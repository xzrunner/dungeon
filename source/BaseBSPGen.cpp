#include "dungeon/BaseBSPGen.h"

#include <queue>

namespace
{

float rand01()
{
    return (float)rand() / (float)RAND_MAX;
}

const size_t MAX_ITER = 65535;

}

namespace dungeon
{

BaseBSPGen::BaseBSPGen(const sm::vec2& size,
                       const sm::vec2& split_sz,
                       const sm::vec2& min_sz,
                       uint32_t seed)
    : m_seed(seed)
{
    srand(seed);
    BuildBSP(size, split_sz, min_sz);
    BuildDungeon(min_sz / 2);
}

std::vector<sm::rect>
BaseBSPGen::GetAllRooms() const
{
    std::vector<sm::rect> rooms;
    if (!m_root) {
        return rooms;
    }

    std::queue<const Node*> buf;
    buf.push(m_root.get());
    while (!buf.empty())
    {
        auto n = buf.front(); buf.pop();
        if (n->m_split == SplitType::None) {
            rooms.push_back(n->m_room);
        } else {
            buf.push(n->m_kids[0].get());
            buf.push(n->m_kids[1].get());
        }
    }

    return rooms;
}

void BaseBSPGen::BuildBSP(const sm::vec2& size,
                          const sm::vec2& split_sz,
                          const sm::vec2& min_sz)
{
    if (split_sz.x < size.x || split_sz.y < size.y) {
        m_root = std::make_unique<Node>(sm::rect(0, 0, size.x, size.y));
        m_root->Split(split_sz, min_sz);
    }
}

void BaseBSPGen::BuildDungeon(const sm::vec2& min_sz)
{
    if (!m_root) {
        return;
    }

    std::queue<Node*> buf;
    buf.push(m_root.get());
    while (!buf.empty())
    {
        auto n = buf.front(); buf.pop();
        if (n->m_split == SplitType::None)
        {
            size_t itr = 0;
            do {
                n->m_room.xmin = n->m_rect.xmin + (n->m_rect.xmax - n->m_rect.xmin) * rand01();
                n->m_room.xmax = n->m_room.xmin + (n->m_rect.xmax - n->m_room.xmin) * rand01();
                n->m_room.ymin = n->m_rect.ymin + (n->m_rect.ymax - n->m_rect.ymin) * rand01();
                n->m_room.ymax = n->m_room.ymin + (n->m_rect.ymax - n->m_room.ymin) * rand01();
            } while ((n->m_room.Width() < min_sz.x || n->m_room.Height() < min_sz.y) && ++itr < MAX_ITER);
        }
        else
        {
            buf.push(n->m_kids[0].get());
            buf.push(n->m_kids[1].get());
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// class BaseBSPGen::Node
//////////////////////////////////////////////////////////////////////////

BaseBSPGen::Node::Node(const sm::rect& r)
    : m_rect(r)
{
}

bool BaseBSPGen::Node::Split(const sm::vec2& split_sz,
                             const sm::vec2& min_sz)
{
    float sz_x = m_rect.Width();
    float sz_y = m_rect.Height();
    if (sz_x >= split_sz.x && sz_y >= split_sz.y) {
        if (rand() % 2 == 0) {
            m_split = SplitType::Hori;
        } else {
            m_split = SplitType::Vert;
        }
    } else if (sz_x >= split_sz.x) {
        m_split = SplitType::Hori;
    } else if (sz_y >= split_sz.y) {
        m_split = SplitType::Vert;
    } else {
        return false;
    }

    float pos = 0.0f;
    size_t itr = 0;
    do {
        pos = static_cast<float>(rand()) / RAND_MAX;
    } while ((m_split == SplitType::Hori && m_rect.Width() * std::min(pos, 1 - pos) < min_sz.x ||
              m_split == SplitType::Vert && m_rect.Height() * std::min(pos, 1 - pos) < min_sz.y) &&
              ++itr < MAX_ITER);

    if (m_split == SplitType::Hori)
    {
        const float mid_x = m_rect.xmin + (m_rect.xmax - m_rect.xmin) * pos;
        m_kids[0] = std::make_unique<Node>(sm::rect(m_rect.xmin, m_rect.ymin, mid_x, m_rect.ymax));
        m_kids[1] = std::make_unique<Node>(sm::rect(mid_x, m_rect.ymin, m_rect.xmax, m_rect.ymax));
    }
    else
    {
        const float mid_y = m_rect.ymin + (m_rect.ymax - m_rect.ymin) * pos;
        m_kids[0] = std::make_unique<Node>(sm::rect(m_rect.xmin, m_rect.ymin, m_rect.xmax, mid_y));
        m_kids[1] = std::make_unique<Node>(sm::rect(m_rect.xmin, mid_y, m_rect.xmax, m_rect.ymax));
    }

    m_kids[0]->Split(split_sz, min_sz);
    m_kids[1]->Split(split_sz, min_sz);

    return true;
}

}