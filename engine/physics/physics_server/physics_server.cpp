#include "engine/physics/physics_server/physics_server.h"
#include "engine/core/core_components/components.h"
#include "game/src/voxel/voxel.h"
#include <GLFW/glfw3.h>

#include <utility> //for swap

//////////////////////////////////////////////////////////////////// Random helpers ///////////////////////////////////////////////////////////////////////////////

bool t = true;

bool debugMode(bool t)
{
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_P) == GLFW_PRESS)
    {
        return !t;
    }
    return t;
}

///////////////////////////////////////////////////////////////// Inertia calculations /////////////////////////////////////////////////////////////////////////

// Basically store the inertia in a matrix to allow for tensor product
glm::mat3 computeInertiaTensor(GameObjectData *obj)
{
    auto *collider = obj->getComponent<C_Collider>();
    auto *body = obj->getComponent<C_RigidBody>();
    auto *transform = obj->getComponent<C_Transform>();

    // For now we'll just say that all planes must be static idk
    if (body->isStatic)
    {
        return glm::mat3(99999.0f);
    }

    if (collider->collider.base.type == SPHERE)
    {
        float r = collider->collider.sphere.radius * transform->getGlobalScale().x;
        float I = (2.0f / 5.0f) * body->mass * r * r;
        return glm::mat3(
            I, 0.0f, 0.0f,
            0.0f, I, 0.0f,
            0.0f, 0.0f, I);
    }

    return glm::mat3(0.0f);
    // TODO : Handle other shapes, i guess rn rn we dont really give a fuck about other shapes
}

// Inverse of whatever the thing above means
glm::mat3 invInertiaTensor(GameObjectData *obj)
{
    return glm::inverse(computeInertiaTensor(obj));
}

///////////////////////////////////////////////////////////////// Intersection functions /////////////////////////////////////////////////////////////////////////:
// Most math and logic behind the intersections is from here :
// https://github.com/IainWinter/IwEngine/blob/3e2052855fea85718b7a499a7b1a3befd49d812b/IwEngine/include/iw/physics/impl/TestCollision.h#L25

// Sphere Vs Sphere intersection
intersectionData intersectionSphereSphere(const Collider *a, const C_Transform *at,
                                          const Collider *b, const C_Transform *bt)
{

    // std::cout << "Sphere Sphere" << std::endl;

    assert(a->type == ColliderType::SPHERE && b->type == ColliderType::SPHERE);

    const SphereCollider *sphere_a = static_cast<const SphereCollider *>(a);
    const SphereCollider *sphere_b = static_cast<const SphereCollider *>(b);

    intersectionData res;

    glm::vec3 aCenter = sphere_a->center + at->getGlobalPosition();
    glm::vec3 bCenter = sphere_b->center + bt->getGlobalPosition();

    float aRadius = sphere_a->radius * at->getGlobalScale().x;
    float bRadius = sphere_b->radius * bt->getGlobalScale().x;

    glm::vec3 ab = bCenter - aCenter;

    float distance = glm::length(ab);

    if (distance < 0.0001f || distance > aRadius + bRadius)
    {
        res.isIntersection = false;
        return res;
    }

    glm::vec3 normal = glm::normalize(ab);
    glm::vec3 intersectionA = aCenter + normal * aRadius;
    glm::vec3 intersectionB = bCenter - normal * bRadius;

    // std::cout << "Intersection : Sphere Sphere" << std::endl;

    res.isIntersection = true;
    res.intersectionNormal = normal;
    res.intersectionPointA = intersectionA;
    res.intersectionPointB = intersectionB;

    return res;
}

intersectionData intersectionSpherePlane(const Collider *a, const C_Transform *at,
                                         const Collider *b, const C_Transform *bt)
{

    // std::cout << "Sphere Plane" << std::endl;

    assert(a->type == ColliderType::SPHERE && b->type == ColliderType::PLANE);

    intersectionData res;

    const SphereCollider *sphere = static_cast<const SphereCollider *>(a);
    const PlaneCollider *plane = static_cast<const PlaneCollider *>(b);

    glm::vec3 sphereCenter = sphere->center + at->getGlobalPosition();
    float sphereRadius = sphere->radius * at->getGlobalScale().x; // Assuming our scale is uniform, if not juste use the max

    glm::vec3 planeNormal = glm::normalize(bt->getGlobalRotationQuat() * plane->normal); // With rotations
    // glm::vec3 planeNormal = glm::normalize(plane->normal); // without
    glm::vec3 planeNormal2 = plane->normal;

    float planeDistance = plane->distance;
    glm::vec3 onPlane = planeNormal * planeDistance + bt->getGlobalPosition();

    float distance = glm::dot(sphereCenter - onPlane, planeNormal);
    // std::cout << "----------------------------------------------------------------" << std::endl;
    // std::cout << "planeNormal (not normalized): (" << planeNormal2.x << ", " << planeNormal2.y << ", " << planeNormal2.z << ")" << std::endl;
    // std::cout << "planeNormal : (" << planeNormal.x << ", " << planeNormal.y << ", " << planeNormal.z << ")" << std::endl;
    // std::cout << "plane global position: (" << bt->getGlobalPosition().x << ", " << bt->getGlobalPosition().y << ", " << bt->getGlobalPosition().z << ")" << std::endl;
    // std::cout << "sphere global position : (" << at->getGlobalPosition().x << ", " << at->getGlobalPosition().y << ", " << at->getGlobalPosition().z << ")" << std::endl;
    // std::cout << "onPlane : (" << onPlane.x << ", " << onPlane.y << ", " << onPlane.z << ")" << std::endl;
    // std::cout << "distance : " << distance << std::endl;
    // std::cout << "radius : " << sphereRadius << std::endl;
    if (distance > sphereRadius)
    {
        res.isIntersection = false;
        return res;
    }

    glm::vec3 intersectionPointB = sphereCenter - planeNormal * distance; //* (2.0f * (glm::dot(planeNormal, sphereCenter - onPlane) < 0) - 1.0f);
    glm::vec3 intersectionPointA = sphereCenter - planeNormal * sphereRadius;

    // std::cout << "Intersection : Sphere Plane" << std::endl;
    res.isIntersection = true;
    res.intersectionPointA = intersectionPointA;
    res.intersectionPointB = intersectionPointB;
    res.intersectionNormal = planeNormal;

    return res;
}

// https://gamedev.stackexchange.com/questions/156870/how-do-i-implement-a-aabb-sphere-collision
//  We use the square distance from the closest point to compare the cube

intersectionData intersectionSphereCube(const Collider *a, const C_Transform *at,
                                        const Collider *b, const C_Transform *bt)
{
    assert(a->type == ColliderType::SPHERE && b->type == ColliderType::CUBE);

    intersectionData res;

    const SphereCollider *sphere = static_cast<const SphereCollider *>(a);
    const CubeCollider *cube = static_cast<const CubeCollider *>(b);

    // sphere to world
    glm::vec3 sphereCenter = sphere->center + at->getGlobalPosition();
    float sphereRadius = sphere->radius * at->getGlobalScale().x; // Assuming our scale is uniform, if not just use the max

    // cube to world
    glm::vec3 cubeCenter = cube->center + bt->getGlobalPosition();
    glm::vec3 halfSize = cube->halfSize * bt->getGlobalScale(); // I think this scales this properly
    glm::vec3 cubeMin = cubeCenter - halfSize;
    glm::vec3 cubeMax = cubeCenter + halfSize;

    // closest points : https://gdbooks.gitbooks.io/3dcollisions/content/Chapter1/closest_point_aabb.html
    glm::vec3 closestPoint = glm::clamp(sphereCenter, cubeMin, cubeMax);

    // TODO : Remove if the clam works
    // glm::vec3 closestPoint;
    // for (int i = 0; i < 3; i++) {
    //     float val = sphereCenter[i];
    //     if (val < cubeMin[i]) val = cubeMin[i];
    //     if (val > cubeMax[i]) val = cubeMax[i];
    //     closestPoint[i] = val;
    // }

    // square distance, not 100% sure but pretty sure this should work
    float sqDist = glm::dot(sphereCenter - closestPoint, sphereCenter - closestPoint);

    if (sqDist > sphere->radius * sphere->radius)
    {
        res.isIntersection = false;
        return res;
    }

    // std::cout << "==========================================================================================" << std::endl;
    // std::cout << "Sphere Cube" << std::endl;
    glm::vec3 intersectionNormal;
    if (closestPoint == sphereCenter)
    {
        std::cout << "Sphere Transform: Position (" << at->getGlobalPosition().x << ", " << at->getGlobalPosition().y << ", " << at->getGlobalPosition().z << "), Scale (" << at->getGlobalScale().x << ", " << at->getGlobalScale().y << ", " << at->getGlobalScale().z << ")" << std::endl;
        std::cout << "Cube Transform: Position (" << bt->getGlobalPosition().x << ", " << bt->getGlobalPosition().y << ", " << bt->getGlobalPosition().z << "), Scale (" << bt->getGlobalScale().x << ", " << bt->getGlobalScale().y << ", " << bt->getGlobalScale().z << ")" << std::endl;
        std::cout << "equals" << std::endl;
        intersectionNormal = glm::normalize(sphereCenter - cubeCenter); // to avoid 0s or nan cause i kept cetting those
        t = true;
    }
    else
        intersectionNormal = glm::normalize(sphereCenter - closestPoint);                 // Standard case normally
    glm::vec3 sphereIntersectionPoint = sphereCenter + intersectionNormal * sphereRadius; // FIXME NO CLUE IF THIS IS RIGHT

    res.isIntersection = true;
    res.intersectionPointA = sphereIntersectionPoint;
    res.intersectionPointB = closestPoint;
    res.intersectionNormal = intersectionNormal;
    // std::cout << "==========================================================================================" << std::endl;
    // std::cout << "Sphere Cube" << std::endl;
    // std::cout << "normal (: " << res.intersectionNormal.x << "," << res.intersectionNormal.y << "," << res.intersectionNormal.z << ")" << std::endl;
    // std::cout << "intersectionPointA (: " << res.intersectionPointA.x << "," << res.intersectionPointA.y << "," << res.intersectionPointA.z << ")" << std::endl;
    // std::cout << "intersectionPointB (: " << res.intersectionPointB.x << "," << res.intersectionPointB.y << "," << res.intersectionPointB.z << ")" << std::endl;

    // t = true;
    return res;
}

intersectionData intersectionPlanePlane(const Collider *a, const C_Transform *at,
                                        const Collider *b, const C_Transform *bt)
{

    // std::cout << "Plane Plane" << std::endl;

    assert(a->type == ColliderType::PLANE && b->type == ColliderType::PLANE);

    const PlaneCollider *aPlane = static_cast<const PlaneCollider *>(a);
    const PlaneCollider *bPlane = static_cast<const PlaneCollider *>(b);

    intersectionData res;

    glm::vec3 aNormal = glm::normalize(at->getGlobalRotationQuat() * aPlane->normal);
    glm::vec3 bNormal = glm::normalize(bt->getGlobalRotationQuat() * bPlane->normal);

    float aDistance = aPlane->distance;
    float bDistance = bPlane->distance;

    glm::vec3 direction = glm::cross(aNormal, bNormal);

    if (glm::length(direction) < 0.0001f)
    {
        res.isIntersection = false;
        return res;
    }

    // Find a point on the line of intersection
    float det = glm::dot(aNormal, glm::cross(bNormal, direction));
    if (fabs(det) < 0.0001f)
    {
        res.isIntersection = false;
        return res;
    }

    glm::vec3 pointOnLine = ((bDistance * glm::cross(direction, aNormal)) +
                             (aDistance * glm::cross(bNormal, direction))) /
                            det;

    // std::cout << "Intersection : Plane Plane" << std::endl;

    res.isIntersection = true;
    res.intersectionNormal = glm::normalize(direction);
    res.intersectionPointA = pointOnLine;
    res.intersectionPointB = pointOnLine;

    return res;
}

// https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html
intersectionData intersectionPlaneCube(const Collider *a, const C_Transform *at,
                                       const Collider *b, const C_Transform *bt)
{
    assert(a->type == ColliderType::PLANE && b->type == ColliderType::CUBE);

    const PlaneCollider *plane = static_cast<const PlaneCollider *>(a);
    const CubeCollider *cube = static_cast<const CubeCollider *>(b);

    intersectionData res;

    glm::vec3 planeNormal = glm::normalize(at->getGlobalRotationQuat() * plane->normal); // TODO : adapt to rotation
    float planeDistance = plane->distance;

    glm::vec3 cubeCenter = cube->center + bt->getGlobalPosition();
    glm::vec3 halfSize = cube->halfSize * bt->getGlobalScale(); // I think this scales this properly
    glm::vec3 cubeMin = cubeCenter - halfSize;
    glm::vec3 cubeMax = cubeCenter + halfSize;

    // check if any of the cube's vertices are on the plane
    bool isIntersecting = false;
    glm::vec3 intersectionPoint;

    bool hasFront = false, hasBack = false;
    float minDistance = FLT_MAX;
    glm::vec3 closestPoint;

    for (auto vertex : cube->getVertices())
    {
        // vertex += bt->getGlobalPosition(); // Might actually just not handle scale
        glm::vec3 worldVertex = vertex * bt->getGlobalScale() + bt->getGlobalPosition();
        float distance = glm::dot(planeNormal, worldVertex) - planeDistance;

        if (distance > 0)
            hasFront = true;
        else if (distance < 0)
            hasBack = true;

        if (std::abs(distance) < minDistance)
        {
            minDistance = std::abs(distance);
            closestPoint = worldVertex;
        }

        if (hasFront && hasBack)
            break;
    }

    if (!(hasFront && hasBack))
    {
        res.isIntersection = false;
        return res;
    }

    res.isIntersection = true;
    res.intersectionNormal = planeNormal;
    res.intersectionPointA = closestPoint; // Closest point on the cube to the plane
    res.intersectionPointB = closestPoint; // Same point for both A and B in this case
    // std::cout << "==========================================================================================" << std::endl;
    // std::cout << "Plane Cube" << std::endl;
    // std::cout << "normal (: " << res.intersectionNormal.x << "," << res.intersectionNormal.y << "," << res.intersectionNormal.z << ")" << std::endl;
    // std::cout << "intersectionPointA (: " << res.intersectionPointA.x << "," << res.intersectionPointA.y << "," << res.intersectionPointA.z << ")" << std::endl;
    // std::cout << "intersectionPointB (: " << res.intersectionPointB.x << "," << res.intersectionPointB.y << "," << res.intersectionPointB.z << ")" << std::endl;

    t = true;

    return res;
}

intersectionData intersectionCubeCube(const Collider *a, const C_Transform *at,
                                      const Collider *b, const C_Transform *bt)
{
    assert(a->type == ColliderType::CUBE && b->type == ColliderType::CUBE);

    const CubeCollider *aCube = static_cast<const CubeCollider *>(a);
    const CubeCollider *bCube = static_cast<const CubeCollider *>(b);

    intersectionData res;

    // first cube to world
    glm::vec3 aCubeCenter = aCube->center + at->getGlobalPosition();
    glm::vec3 aHalfSize = aCube->halfSize * at->getGlobalScale();
    glm::vec3 aCubeMin = aCubeCenter - aHalfSize;
    glm::vec3 aCubeMax = aCubeCenter + aHalfSize;

    // second cube to world
    glm::vec3 bCubeCenter = bCube->center + bt->getGlobalPosition();
    glm::vec3 bHalfSize = bCube->halfSize * bt->getGlobalScale();
    glm::vec3 bCubeMin = bCubeCenter - bHalfSize;
    glm::vec3 bCubeMax = bCubeCenter + bHalfSize;

    // check if intere
    if (!(aCubeMin.x <= bCubeMax.x && aCubeMax.x >= bCubeMin.x) ||
        !(aCubeMin.y <= bCubeMax.y && aCubeMax.y >= bCubeMin.y) ||
        !(aCubeMin.z <= bCubeMax.z && aCubeMax.z >= bCubeMin.z))
    {
        res.isIntersection = false;
        return res;
    }

    // Closest points on each cube
    glm::vec3 contactPointA = glm::clamp(bCubeCenter, aCubeMin, aCubeMax);
    glm::vec3 contactPointB = glm::clamp(aCubeCenter, bCubeMin, bCubeMax);

    res.isIntersection = true;
    res.intersectionNormal = glm::normalize(bCubeCenter - aCubeCenter); // i think that's right idk
    res.intersectionPointA = contactPointA;
    res.intersectionPointB = contactPointB;

    // std::cout << "==========================================================================================" << std::endl;
    // std::cout << "Cube Cube" << std::endl;
    // std::cout << "normal (: " << res.intersectionNormal.x << "," << res.intersectionNormal.y << "," << res.intersectionNormal.z << ")" << std::endl;
    // std::cout << "intersectionPointA (: " << res.intersectionPointA.x << "," << res.intersectionPointA.y << "," << res.intersectionPointA.z << ")" << std::endl;
    // std::cout << "intersectionPointB (: " << res.intersectionPointB.x << "," << res.intersectionPointB.y << "," << res.intersectionPointB.z << ")" << std::endl;

    // t = true;

    return res;
}

// CHUNK


intersectionData intersectionSphereChunk(const Collider *a, const C_Transform *at,
                                         const Collider *b, const C_Transform *bt)
{

    // Types secrutiy checks + casts
    assert(a->type == ColliderType::SPHERE && b->type == ColliderType::CHUNK);

    const SphereCollider *sphere = static_cast<const SphereCollider *>(a);
    const ChunkCollider *chunk = static_cast<const ChunkCollider *>(b);

    intersectionData res;

    // Sphere to world
    glm::vec3 sphereCenter = sphere->center + at->getGlobalPosition();
    float sphereRadius = sphere->radius * at->getGlobalScale().x;

    // (variables to make less annoying)
    const C_voxelMesh *mesh = chunk->voxelMesh;
    const VoxelContainer &container = mesh->container; 
    
    // Chunk to world
    glm::vec3 chunkOrigin = bt->getGlobalPosition();
    // Utils::print(chunkOrigin);
    float voxelSize = mesh->size;

    // TODO (maybe) add a lil check to make sure we're in a chunk and/or neighbors to avoid useless computation

    // Ok so basically, get the bounding box of my sphere, and just check the voxels inside it
    // This cuts the number of computations by like a bajillion probably

    // Sphere bounding box edges
    glm::vec3 sphereBBMin = sphereCenter - glm::vec3(sphereRadius);
    glm::vec3 sphereBBMax = sphereCenter + glm::vec3(sphereRadius);

    // indexes of our toing (contained between 0 and max size pretty much)
    // floor and ceil are just to make sure we take any voxel even that's barely in

    glm::ivec3 minIdx = glm::clamp(glm::floor((sphereBBMin - chunkOrigin) / voxelSize), glm::vec3(0), glm::vec3(container.sX , container.sY , container.sZ ));
    glm::ivec3 maxIdx = glm::clamp(glm::ceil((sphereBBMax - chunkOrigin) / voxelSize), glm::vec3(0), glm::vec3(container.sX , container.sY , container.sZ ));

    // Utils::print("min_idx : ", minIdx, " max_idx :", maxIdx);

    for (int x = minIdx.x; x < maxIdx.x; ++x)
        for (int y = minIdx.y; y < maxIdx.y; ++y)
            for (int z = minIdx.z; z < maxIdx.z; ++z)

    // for (int x = 0; x < container.sX; ++x)
    //     for (int y = 0; y < container.sY; ++y)
    //         for (int z = 0; z < container.sZ; ++z)
            {
                // Utils::print("in loop : ",x,y,z);
                // just check solids
                VOXEL_IDX_TYPE id = container.get(x, y, z);
                if (id == 0)
                    continue;
                
                // not 100% sure but i think this is ok (not sure of the halfstep or no)
                glm::vec3 voxelMin = chunkOrigin + voxelSize * glm::vec3(x, y, z);
                glm::vec3 voxelMax = voxelMin + glm::vec3(voxelSize);
                glm::vec3 voxelCenter = voxelMin + glm::vec3(voxelSize / 2.0f);

                // Straight up just SphereCube but reskin

                glm::vec3 closestPoint = glm::clamp(sphereCenter, voxelMin, voxelMax);
                float sqDist = glm::dot(sphereCenter - closestPoint, sphereCenter - closestPoint);

                if (sqDist <= sphereRadius * sphereRadius)
                {
                    Utils::print("sqDist : " , sqDist);
                    Utils::print("r2 : " , sphereRadius * sphereRadius);
                    // TODO : add the check to make sure it doesnt return NAN but the sun is rising tbh

                    glm::vec3 intersectionNormal;
                    if (closestPoint == sphereCenter)
                    {
                        intersectionNormal = glm::normalize(sphereCenter - voxelCenter); // to avoid 0s or nan cause i kept cetting those
                        t = true;
                    }
                    else
                        intersectionNormal = glm::normalize(sphereCenter - closestPoint);   
                    glm::vec3 sphereIntersectionPoint = sphereCenter - intersectionNormal * sphereRadius; // FIXME NO CLUE IF THIS IS RIGHT

                    Utils::print("Normal : ",intersectionNormal);

                    res.isIntersection = true;
                    res.intersectionNormal = intersectionNormal;
                    res.intersectionPointA = sphereIntersectionPoint;
                    res.intersectionPointB = closestPoint;
                    return res; // Return on first intersection
                }
            }

            // no intersection
            res.isIntersection = false;
            return res;
}

///////////////////////////////////////////////////////////////// Physics server methods ////////////////////////////////////////////////////////////////////////////

void PhysicsServer::addObject(GameObjectData *go)
{
    Objects.insert(go);
}

void PhysicsServer::removeObject(GameObjectData *go)
{
    Objects.erase(go);
}

float PhysicsServer::getDeltaTime()
{
    return deltaT;
}

std::vector<intersectionData> PhysicsServer::computeCollisions()
{
    std::vector<intersectionData> intersectionList;

    // Check this out to understand this
    // https://winter.dev/articles/physics-engine

    using FindContactFunc = intersectionData (*)(const Collider *, const C_Transform *, const Collider *, const C_Transform *);

    // Each value represents the collision shape in the struct
    static const FindContactFunc functionTables[5][5] = {
        {nullptr, nullptr, nullptr, nullptr},
        {nullptr, intersectionSphereSphere, intersectionSpherePlane, intersectionSphereCube, intersectionSphereChunk},
        {nullptr, nullptr, intersectionPlanePlane, intersectionPlaneCube, nullptr},
        {nullptr, nullptr, nullptr, intersectionCubeCube, nullptr},
        {nullptr, nullptr, nullptr, nullptr, nullptr}};

    for (const auto &objectA : Objects)
    {
        if (!objectA->hasComponent<C_Collider>() || !objectA->hasComponent<C_RigidBody>())
            continue;
        for (const auto &objectB : Objects)
        {

            if (objectA->getComponent<C_RigidBody>()->isStatic && objectB->getComponent<C_RigidBody>()->isStatic)
                continue;

            // Check if the object has the right components AND that they're not the same object
            if (
                !objectB->hasComponent<C_Collider>() ||
                !objectB->hasComponent<C_RigidBody>() ||
                objectA->getId() <= objectB->getId())
                continue;

            intersectionData intersection;

            // check if it can work in our table, if not, flip it

            Collider *colliderA = &objectA->getComponent<C_Collider>()->collider.base;
            C_Transform *transformA = objectA->getComponent<C_Transform>();

            Collider *colliderB = &objectB->getComponent<C_Collider>()->collider.base;
            C_Transform *transformB = objectB->getComponent<C_Transform>();

            bool swap = (objectB->getComponent<C_Collider>()->collider.base.type) < (objectA->getComponent<C_Collider>()->collider.base.type);

            // flip it (or not)
            if (swap)
            {

                // safety check
                //  std::cout << "id 1st : " << objectB->getId() << std::endl;
                //  std::cout << "id 2nd : " << objectA->getId() << std::endl;
                auto func = functionTables[colliderB->type][colliderA->type];
                if (!func)
                {
                    std::cout << "func[" << colliderB->type << "][" << colliderA->type << "] does not exist" << std::endl;
                    continue;
                }

                intersection = func(colliderB, transformB, colliderA, transformA);
                intersection.objectA = objectB;
                intersection.objectB = objectA;
            }
            else
            {

                // same safety check
                //  std::cout << "id 1st : " << objectA->getId() << std::endl;
                //  std::cout << "id 2nd : " << objectB->getId() << std::endl;
                auto func = functionTables[colliderA->type][colliderB->type];
                if (!func)
                {
                    std::cout << "func[" << colliderA->type << "][" << colliderB->type << "] does not exist" << std::endl;
                    continue;
                }

                intersection = func(colliderA, transformA, colliderB, transformB);
                intersection.colliderA = colliderA;
                intersection.colliderB = colliderB;
                intersection.objectA = objectA;
                intersection.objectB = objectB;
            }

            intersectionList.push_back(intersection);
        }
    }
    return intersectionList;
}

void solveOverlap(intersectionData &a){

    static const float MARGIN = 0.000001f;

    auto *objectA = a.objectA;
    auto *objectB = a.objectB;

    float marginA = MARGIN, marginB = MARGIN;

    glm::vec3 overlapVector(a.intersectionPointB - a.intersectionPointA);

    float mass_ratio;
    if (objectB->getComponent<C_RigidBody>()->isStatic){
        mass_ratio = 1.0;
        marginB = 0.0f;
    } else if (objectA->getComponent<C_RigidBody>()->isStatic){
        mass_ratio = 0.0;
        marginA = 0.0f;
    } else{
        mass_ratio = objectB->getComponent<C_RigidBody>()->mass / (objectB->getComponent<C_RigidBody>()->mass + objectA->getComponent<C_RigidBody>()->mass);
    }

    auto v = objectA->getComponent<C_Transform>()->getGlobalPosition();
    objectA->getComponent<C_Transform>()->moveGlobal(overlapVector * (mass_ratio + MARGIN));

    objectB->getComponent<C_Transform>()->moveGlobal(-1.0f * overlapVector  * (1.0f - mass_ratio + MARGIN));
    
    a.intersectionPointA = a.intersectionPointB = a.intersectionPointA + (mass_ratio) * overlapVector;
}

// this is like almost word for word this :
// https://github.com/DallinClark/3d-physics-engine/blob/main/src/physics/world.cpp
void PhysicsServer::resolveCollision(const intersectionData &a)
{
    // Objects
    auto *objectA = a.objectA;
    auto *objectBodyA = objectA->getComponent<C_RigidBody>();
    auto *objectTransformA = objectA->getComponent<C_Transform>();

    auto *objectB = a.objectB;
    auto *objectBodyB = objectB->getComponent<C_RigidBody>();
    auto *objectTransformB = objectB->getComponent<C_Transform>();

    if (objectBodyA->isStatic && objectBodyB->isStatic)
        return;

    // Intersection normal
    glm::vec3 normal = -a.intersectionNormal;

    // Points of collision
    glm::vec3 collisionPoint = a.intersectionPointA; // same

    // Smalerst restitution value (take the min elasticity)
    float e = std::min(objectBodyA->restitution, objectBodyB->restitution);

    // Collision offsets
    // ngl i'm not sure which collision points i should be using
    glm::vec3 ra = collisionPoint - objectTransformA->getGlobalPosition();
    glm::vec3 rb = collisionPoint - objectTransformB->getGlobalPosition();

    // Calculate new angular velocity based on the offset
    glm::vec3 angularVelocityA = glm::cross(objectBodyA->angular_velocity, ra);
    glm::vec3 angularVelocityB = glm::cross(objectBodyB->angular_velocity, rb);

    // Relative velocity at the points of contact
    glm::vec3 relativeVelocity = (objectBodyB->linear_velocity + angularVelocityB) - (objectBodyA->linear_velocity + angularVelocityA);

    // Magnitude of contact veloziwty
    float contactVelocityMag = glm::dot(relativeVelocity, normal);

    Utils::print("hello??");

    // If < 0 means colliusion, if no collision, give empty impulse
    float j = 0.0f;
    if (contactVelocityMag <= 0.0f)
    {
        // Inertia
        glm::vec3 raInertia = invInertiaTensor(objectA) * glm::cross(ra, normal);
        if (objectBodyA->isStatic)
        {
            raInertia = glm::vec3(0.0f);
        }
        glm::vec3 rbInertia = invInertiaTensor(objectB) * glm::cross(rb, normal);
        if (objectBodyB->isStatic)
        {
            rbInertia = glm::vec3(0.0f);
        }

        // Denominator combines the inverse masses and the rotational inertia terms.
        float denom = objectBodyA->inverseMass() + objectBodyB->inverseMass() + glm::dot(glm::cross(raInertia, ra), normal) + glm::dot(glm::cross(rbInertia, rb), normal);

        // Compute the impulse scalar.
        j =  -e * contactVelocityMag / denom;

        glm::vec3 impulse = j * normal / 2.0f;

        // Update linear velocity
        // No need to check if static cuz static bodies have an inverseMass of 0
        Utils::print("impulse : " , impulse);
        Utils::print("add impulse : " , objectBodyA->linear_velocity - impulse * objectBodyA->inverseMass());
        Utils::print("velocity before : " , objectBodyA->linear_velocity);
        objectBodyA->applyImpulse(-1.0f * impulse * objectBodyA->inverseMass());
        objectBodyB->applyImpulse(impulse * objectBodyB->inverseMass());
        Utils::print("velocity after : ", objectBodyA->linear_velocity);

        objectBodyA->linear_velocity -= (gravity * objectBodyA->gravityScale + objectBodyA->acceleration) * getDeltaTime();
        objectBodyB->linear_velocity -= (gravity * objectBodyB->gravityScale + objectBodyB->acceleration) * getDeltaTime();


        glm::vec3 angularImpulseA = invInertiaTensor(objectA) * glm::cross(ra, impulse);
        if (objectBodyA->isStatic)
        {
            angularImpulseA = glm::vec3(0.0f);
        }
        glm::vec3 angularImpulseB = invInertiaTensor(objectB) * glm::cross(rb, impulse);
        if (objectBodyB->isStatic)
        {
            angularImpulseB = glm::vec3(0.0f);
        }



        // Update angular velocities accordingly.
        objectBodyA->angular_velocity = (objectBodyA->angular_velocity - angularImpulseA);
        objectBodyB->angular_velocity = (objectBodyB->angular_velocity - angularImpulseB);
    }

}

void PhysicsServer::solveCollisions(std::vector<intersectionData> collisions, float deltatime)
{
    for (auto &collision : collisions)
    {
        if (collision.isIntersection)
        {
            solveOverlap(collision);
            resolveCollision(collision);

            auto *objectBodyA = collision.objectA->getComponent<C_RigidBody>();
            auto *objectBodyB = collision.objectB->getComponent<C_RigidBody>();

            // collision.objectA->getComponent<C_RigidBody>()->linear_velocity -= (gravity * objectBodyA->gravityScale + objectBodyA->acceleration) * deltatime;
            // collision.objectB->getComponent<C_RigidBody>()->linear_velocity -= (gravity * objectBodyB->gravityScale + objectBodyB->acceleration) * deltatime;

            float THRESH = 0.0005f;
            if (glm::dot(collision.objectA->getComponent<C_RigidBody>()->linear_velocity, collision.objectA->getComponent<C_RigidBody>()->linear_velocity) < THRESH)
                collision.objectA->getComponent<C_RigidBody>()->linear_velocity = glm::vec3(0.0);
            if (glm::dot(collision.objectB->getComponent<C_RigidBody>()->linear_velocity, collision.objectB->getComponent<C_RigidBody>()->linear_velocity) < THRESH)
                collision.objectB->getComponent<C_RigidBody>()->linear_velocity = glm::vec3(0.0);

            // t = true;
        }
    }
}

// Source is this book :
// https://books.google.fr/books?hl=en&lr=&id=dSbMBQAAQBAJ&oi=fnd&pg=PP1&dq=3d+physics+engine&ots=qe2HUKonLp&sig=JMjj1CI9Jnisktw-MRhHuGTIczg&redir_esc=y#v=onepage&q&f=true
void PhysicsServer::integrate(float deltatime)
{
    for (const auto &object : Objects)
    {
        // If the obj is not static apply physics
        if (object->getComponent<C_RigidBody>()->isStatic)
            continue;

        auto *objectBody = object->getComponent<C_RigidBody>();

        // https://github.com/DallinClark/3d-physics-engine/blob/main/src/physics/rigid_body.cpp
        // If we have angular velocity and we're not an aabb
        if (glm::length(objectBody->angular_velocity) > 0.0f && !(object->getComponent<C_Collider>()->collider.base.type == CUBE))
        {

            float angle = glm::length(objectBody->angular_velocity) * deltatime; // Get the rotation magnitude
            glm::vec3 axis = glm::normalize(objectBody->angular_velocity);       // The rotation axis

            // Construct the small-angle rotation matrix
            //glm::mat4 deltaRotation = glm::rotate(glm::mat4(1.0f), angle, axis);

            // Convert rotation matrix to euler tho idk if that's necessary at all
            // FIXME
            //glm::vec3 eulerAngles = (angle);
            
            // Apply rotation update
            object->getComponent<C_Transform>()->rotate(objectBody->angular_velocity * deltatime);
            objectBody->angular_velocity *= pow(objectBody->damping, deltatime);
            
        }

        // update de velocity
        objectBody->linear_velocity += (gravity * objectBody->gravityScale + objectBody->acceleration) * deltatime;
        objectBody->linear_velocity *= pow(objectBody->damping, deltatime);

        object->getComponent<C_Transform>()->moveGlobal(objectBody->linear_velocity * gameSpeed);
    }
}

void PhysicsServer::step(float deltatime)
{
    deltaT = deltatime * 1;
    // t = debugMode(t);
    t = false;
    if (t)
        return;
    integrate(deltaT);
    std::vector<intersectionData> collisions = computeCollisions();
    solveCollisions(collisions, deltaT);
}
