//
// Created by Vlad on 28.07.2024.
//

#pragma once

#include "omath/Vector3.h"
#include <expected>
#include <vector>
#include <string>


namespace omath::pathfinding
{
    struct NavigationVertex
    {
        Vector3 origin;
        std::vector<NavigationVertex*> connections;
    };


    class NavigationMesh final
    {
    public:

        [[nodiscard]]
        std::expected<Vector3, std::string> GetClossestVertex(const Vector3& point) const;


        [[nodiscard]]
        const std::vector<Vector3>& GetNeighbors(const Vector3& vertex) const;

        [[nodiscard]] std::vector<uint8_t> Serialize() const;
        void Deserialize(const std::vector<uint8_t>& raw);
        
        std::unordered_map<Vector3, std::vector<Vector3>> m_verTextMap;
    };
}