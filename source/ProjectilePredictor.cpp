//
// Created by vlad on 11/6/23.
//

#include "uml/ProjectilePredictor.h"
#include "uml/Vector3.h"
#include "uml/angles.h"

#include <cmath>
#include <cstdio>

namespace uml::prediction
{
    ProjectilePredictor::ProjectilePredictor(float gravityValue,
                                             float maxTimeToTravel,
                                             float timeStep)
    : m_gravity(gravityValue), m_maxTravelTime(maxTimeToTravel), m_timeStepSize(timeStep)
    {

    }

    std::optional<Vector3> ProjectilePredictor::PredictPointToAim(
            const Target &target, const Projectile &projectile) const
    {
        for (float time = 0.0f; time <= m_maxTravelTime; time += m_timeStepSize)
        {
            auto predictedTargetPosition = LinearPrediction(target, time);

            const auto projectilePitch =
                    MaybeCalculateProjectileLaunchPitchAngle(projectile, predictedTargetPosition);

            if (!projectilePitch.has_value()) [[unlikely]]
                return std::nullopt;

            const auto timeToHit = ProjectileTravelTime(predictedTargetPosition,
                                                        projectile,
                                                        projectilePitch.value());
            if (timeToHit > time)
                continue;

            const auto delta2d = (predictedTargetPosition - projectile.m_origin).Length2D();
            const auto height = delta2d * std::tan(angles::DegToRad(projectilePitch.value()));

            predictedTargetPosition.z = projectile.m_origin.z + height;

            return predictedTargetPosition;
        }


        return std::nullopt;
    }

    Vector3 ProjectilePredictor::LinearPrediction(const Target &target, float time) const
    {
        auto predicted = target.m_origin + target.m_vecVelocity * time;

        if (target.m_IsAirborne)
            predicted.z -= m_gravity * std::pow(time, 2.f) * 0.5f;

        return predicted;
    }

    std::optional<float>
    ProjectilePredictor::MaybeCalculateProjectileLaunchPitchAngle(const Projectile &projectile,
                                                                  const Vector3 &targetPosition)
                                                                  const
    {
        auto bulletGravity = projectile.m_gravityMultiplier*750.f;
        const auto delta = targetPosition - projectile.m_origin;;

        const auto distance2d = delta.Length2D();


        float root = std::pow(projectile.m_velocity, 4.f) - bulletGravity * (bulletGravity *
            std::pow(distance2d, 2.f) + 2.0f * delta.z * std::pow(projectile.m_velocity, 2.f));

        if (root < 0.0f) [[unlikely]]
            return std::nullopt;

        root = std::sqrt(root);
        const float angle = std::atan((std::pow(projectile.m_velocity, 2.f) - root) / (bulletGravity * distance2d));

        return angles::RadToDeg(angle);
    }

    float ProjectilePredictor::ProjectileTravelTime(const Vector3 &end,
                                                    const Projectile &projectile,
                                                    const float angle) const
    {
        auto launchAngles = projectile.m_origin.ViewAngleTo(end);
        launchAngles.x = angle;

        const auto velocity = Vector3::CreateVelocity(launchAngles, projectile.m_velocity);
        auto prevProjectilePosition = projectile.m_origin;

        for (float time = 0.0f; time <= m_maxTravelTime; time += m_timeStepSize)
        {
            auto currentPos = projectile.m_origin + velocity * time;
            currentPos.z -= m_gravity * projectile.m_gravityMultiplier * std::pow(time, 2.f) * 0.5f;

            if (prevProjectilePosition.DistTo(end) < currentPos.DistTo(end))
                return time;

            prevProjectilePosition = currentPos;
        }

        return 0;
    }
}
