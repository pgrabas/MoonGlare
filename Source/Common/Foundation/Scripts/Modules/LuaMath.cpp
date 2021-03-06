#include <cmath>
#include <functional>
#include "Math.h"
#include "StaticModules.h"

#include <bullet/LinearMath/btVector3.h>
#include <bullet/LinearMath/btQuaternion.h>
#include <bullet/LinearMath/btTransform.h>
#include <glm/glm.hpp>
#include <eigen3/Eigen/Core>

#include <Foundation/xMath.h>
#include <Foundation/TemplateUtils.h>

//::Space::RTTI::TypeInfoInitializer<Eigen::Vector2f, Eigen::Vector3f, Eigen::Vector4f, Eigen::Matrix4f> EigenTypeInfo;

namespace MoonGlare::Scripts::Modules {

#define NN(a)   \
if (!a)    __debugbreak();
//throw std::runtime_error(__FUNCTION__ " - nullptr! (" #a ")");

//Other

template<int multiplier = 1,int divider = 1>
float getPi() { return 3.14159f * (static_cast<float>(multiplier) / static_cast<float>(divider)); }

//Templates

template<typename T> inline T VecNormalized(const T * vec) { return glm::normalize(*vec); }
template<typename T> inline void VecNormalize(T *vec) { *vec = glm::normalize(*vec); }
template<typename T> inline float VecLength(const T *vec) { return glm::length(*vec); }
template<typename T> inline float VecDotProduct(T *a, T* b) { return glm::dot(*a, *b); }
template<typename T> inline T VecDiv(T *a, T* b) { NN(a) NN(b) return *a / *b; }
template<typename T> inline T VecMul(T *a, T* b) { NN(a) NN(b) return *a * *b; }
template<typename T> inline T VecAdd(T *a, T* b) { NN(a) NN(b) return *a + *b; }
template<typename T> inline T VecSub(T *a, T* b) { NN(a) NN(b) return *a - *b; }
template<typename T> inline std::string ToString(T *vec) {
    std::ostringstream oss;
    oss << *vec;
    return oss.str();
}
template<typename T, typename A, int ... ints> inline T StaticVec() { return T(static_cast<A>(ints)...); }
template<typename T> inline T VecClamp(const T &v, const T &min, const T &max) {
    //NN(v); NN(min); NN(max);
    T ret = v;
    for (int i = 0; i < ret.length(); ++i) {
        if (ret[i] < min[i])
            ret[i] = min[i];
        else
            if (ret[i] > max[i])
                ret[i] = max[i];
    }
    return ret;
}
template<typename T> inline void VecClampSelf(T *v, const T &min, const T &max) { *v = VecClamp(*v, min, max); }

//Quaternions/Vec4

inline math::vec4 QuaternionCrossProduct(math::vec4 *a, math::vec4* b) {
    auto &q1 = *a;
    auto &q2 = *b;
    math::vec4 out;
    out.x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y;
    out.y = q1.w*q2.y + q1.y*q2.w + q1.z*q2.x - q1.x*q2.z;
    out.z = q1.w*q2.z + q1.z*q2.w + q1.x*q2.y - q1.y*q2.x;
    out.w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
    return out;
}
template<> inline std::string ToString<math::vec4>(math::vec4 *vec) {
    auto &t = *vec;
    char b[256];
    sprintf(b, "Vec4(%f, %f, %f, %f)", t[0], t[1], t[2], t[3]);
    return b;
}
inline math::vec4 QuaternionRotationTo(const math::vec3 &a, const math::vec3 &b) {
    // Based on Stan Melax's article in Game Programming Gems
    math::Quaternion q;
    // Copy, since cannot modify local
    btVector3 v0 = convert(a);
    btVector3 v1 = convert(b);

    v0.normalize();
    v1.normalize();

    float d = v0.dot(v1);
    // If dot == 1, vectors are the same
    if (d >= 1.0f) {
        return convert(math::Quaternion::getIdentity());
    }
    if (d < (1e-6f - 1.0f)) {
        if (v1 != btVector3(0, 0, 0)) {
            // rotate 180 degrees about the fallback axis
            q = math::Quaternion(v1, glm::radians(180.0f));
        } else {
            // Generate an axis
            btVector3  axis = btVector3(1, 0, 0).cross(v0);
            if (axis.length2() < 1e-06f * 1e-06f) // pick another if colinear
                axis = btVector3(0, 1, 0).cross(v0);
            axis.normalize();
            q = math::Quaternion(axis, glm::radians(180.0f));
        }
    } else {
        float s = sqrtf((1 + d) * 2);
        float invs = 1 / s;

        btVector3 c = v0.cross(v1);

        c *= invs;
        q = math::Quaternion(c.getX(), c.getY(), c.getZ(), s * 0.5f);
        q.normalize();
    }
    return convert(q);
}
inline math::vec4 QuaternionFromVec3Angle(const math::vec3 &vec, float a) {
    return convert(math::Quaternion(convert(vec), a));
}
inline math::vec4 QuaternionFromEulerXYZ(float x, float y, float z) {
    return convert(math::Quaternion(x, y, z));
}
inline math::vec4 QuaternionFromAxisAngle(float x, float y, float z, float a) {
    return QuaternionFromVec3Angle(math::vec3(x, y, z), a);
}
inline int lua_NewQuaternion(lua_State *lua) {
    int argc = lua_gettop(lua);
    switch (argc) {
    case 1:
        luabridge::Stack<math::vec4>::push(lua, math::vec4());
        return 1;
    case 2:
        if (lua_isnumber(lua, -1)) {
            luabridge::Stack<math::vec4>::push(lua, math::vec4(static_cast<float>(lua_tonumber(lua, -1))));
            return 1;
        }
        break;
    case 3:
        break;
    case 4:
        break;
    case 5:
        if (lua_isnumber(lua, 2) && lua_isnumber(lua, 3) && lua_isnumber(lua, 4) && lua_isnumber(lua, 5)) {
            luabridge::Stack<math::vec4>::push(lua,
                                               math::vec4(
                                                   static_cast<float>(lua_tonumber(lua, 2)),
                                                   static_cast<float>(lua_tonumber(lua, 3)),
                                                   static_cast<float>(lua_tonumber(lua, 4)),
                                                   static_cast<float>(lua_tonumber(lua, 5))));
            return 1;
        }
    default:
        break;
    }
    AddLog(Error, "Invalid arguments!");
    lua_pushnil(lua);
    return 1;
}

//Vec3

inline math::vec3 Vec3CrossProduct(math::vec3 *a, math::vec3* b) { return glm::cross(*a, *b); }
template<> inline std::string ToString<math::vec3>(math::vec3 *vec) {
    auto &t = *vec;
    char b[256];
    sprintf(b, "Vec3(%f, %f, %f)", t[0], t[1], t[2]);
    return b;
}
inline int lua_NewVec3(lua_State *lua) {
    int argc = lua_gettop(lua);
    switch (argc) {
    case 1:
        luabridge::Stack<math::vec3>::push(lua, math::vec3());
        return 1;
    case 2:
        if (lua_isnumber(lua, -1)) {
            luabridge::Stack<math::vec3>::push(lua, math::vec3(static_cast<float>(lua_tonumber(lua, -1))));
            return 1;
        }

    case 3:
        break;
    case 4:
        if (lua_isnumber(lua, -3) && lua_isnumber(lua, -2) && lua_isnumber(lua, -1)) {
            luabridge::Stack<math::vec3>::push(lua,
                                               math::vec3(
                                                   static_cast<float>(lua_tonumber(lua, -3)),
                                                   static_cast<float>(lua_tonumber(lua, -2)),
                                                   static_cast<float>(lua_tonumber(lua, -1))));
            return 1;
        }
    default:
        break;
    }
    AddLog(Error, "Invalid arguments!");
    lua_pushnil(lua);
    return 1;
}
inline math::vec3 SphericalToCartesian(const math::vec2 *vec) {
    NN(vec)
    float th = vec->y;
    float fi = vec->x;
    return glm::normalize(math::vec3(
        sinf(th) * cosf(fi)
        ,
        sinf(th) * sinf(fi)
        ,
        cos(th)
    ));
}

//Vec2

template<> inline std::string ToString<math::vec2>(math::vec2 *vec) {
    auto &t = *vec;
    char b[256];
    sprintf(b, "vec2(%f, %f)", t[0], t[1]);
    return b;
}
inline int lua_NewVec2(lua_State *lua) {
    int argc = lua_gettop(lua);
    switch (argc) {
    case 1:
        luabridge::Stack<math::vec2>::push(lua, math::vec2());
        return 1;

    case 2:
        if (lua_isnumber(lua, -1)) {
            luabridge::Stack<math::vec2>::push(lua, math::vec2(static_cast<float>(lua_tonumber(lua, -1))));
            return 1;
        }

    case 3:
        if (lua_isnumber(lua, -2) && lua_isnumber(lua, -1)) {
            luabridge::Stack<math::vec2>::push(lua,
                                               math::vec2(
                                                   static_cast<float>(lua_tonumber(lua, -2)),
                                                   static_cast<float>(lua_tonumber(lua, -1))));
            return 1;
        }
    default:
        break;
    }
    AddLog(Error, "Invalid arguments!");
    lua_pushnil(lua);
    return 1;
}

//Linear

inline float Clamp(float v, float min, float max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}
inline float Clamp01(float v) { return Clamp(v, 0.0f, 1.0f); }

//Common

template<typename VEC>
struct VecCommon {
    template<typename API>
    static void f(API &api) {
        api
            .addProperty<VEC>("Normalized", &VecNormalized<VEC>)
            .addProperty<float>("Length", &VecLength<VEC>)

            .addFunction("Normalize", Utils::Template::InstancedStaticCall<VEC, void>::callee<VecNormalize>())
            .addFunction("Clamp", Utils::Template::InstancedStaticCall<VEC, void, const VEC&, const VEC&>::callee<VecClampSelf>())

            .addFunction("__tostring", Utils::Template::InstancedStaticCall<VEC, std::string>::callee<ToString>())
            .addFunction("__mul", Utils::Template::InstancedStaticCall<VEC, VEC, VEC*>::callee<VecMul>())
            .addFunction("__add", Utils::Template::InstancedStaticCall<VEC, VEC, VEC*>::callee<VecAdd>())
            .addFunction("__sub", Utils::Template::InstancedStaticCall<VEC, VEC, VEC*>::callee<VecSub>())
            ;
    }
};

//Registration

void ScriptMathClasses(lua_State *lua) {
    luabridge::getGlobalNamespace(lua)
    .beginNamespace("math")
        .addFunction("Clamp", &Clamp)
        .addFunction("Clamp01", &Clamp01)
    //	.addProperty("pi", &getPi)
    //	.addProperty("pi_half", &getPi<1, 2>)

        .beginClass<math::vec4>("cVec4")
            .addData("x", &math::vec4::x)
            .addData("y", &math::vec4::y)
            .addData("z", &math::vec4::z)
            .addData("w", &math::vec4::w)
            .DefferCalls<&VecCommon<math::vec4>::f>()
        .endClass()
        .beginClass<math::vec3>("cVec3")
            .addData("x", &math::vec3::x)
            .addData("y", &math::vec3::y)
            .addData("z", &math::vec3::z)
            .DefferCalls<&VecCommon<math::vec3>::f>()
        .endClass()
        .beginClass<math::vec2>("cVec2")
            .addData("x", &math::vec2::x)
            .addData("y", &math::vec2::y)
            .DefferCalls<&VecCommon<math::vec2>::f>()
        .endClass()
    .endNamespace()
    ;
}

//-------------------------------------------------------------------------------------------------

void ScriptMathGlobal(lua_State *lua) {
    luabridge::getGlobalNamespace(lua)
    .beginNamespace("Quaternion")
        .addCFunction("__call", &lua_NewQuaternion)

        .addFunction("FromAxisAngle", &QuaternionFromAxisAngle)
        .addFunction("FromVec3Angle", &QuaternionFromVec3Angle)
        .addFunction("FromEulerXYZ", &QuaternionFromEulerXYZ)
        .addFunction("RotationTo", &QuaternionRotationTo)

        .addProperty("Identity", &StaticVec<math::vec4, float, 0, 0, 0, 1>)
    .endNamespace()

    .beginNamespace("Vec4")
        .addCFunction("__call", &lua_NewQuaternion)
        .addFunction("Clamp", &VecClamp<math::vec4>)

        .addProperty("X", &StaticVec<math::vec4, float, 1, 0, 0, 0>)
        .addProperty("Y", &StaticVec<math::vec4, float, 0, 1, 0, 0>)
        .addProperty("Z", &StaticVec<math::vec4, float, 0, 0, 1, 0>)
        .addProperty("W", &StaticVec<math::vec4, float, 0, 0, 0, 1>)

        .addProperty("R", &StaticVec<math::vec4, float, 1, 0, 0, 1>)
        .addProperty("G", &StaticVec<math::vec4, float, 0, 1, 0, 1>)
        .addProperty("B", &StaticVec<math::vec4, float, 0, 0, 1, 1>)
        .addProperty("White", &StaticVec<math::vec4, float, 1, 1, 1, 1>)
        .addProperty("Black", &StaticVec<math::vec4, float, 0, 0, 0, 1>)

        .addProperty("Zero", &StaticVec<math::vec4, float, 0, 0, 0, 0>)
        .addProperty("One", &StaticVec<math::vec4, float, 1, 1, 1, 1>)
    .endNamespace()

    .beginNamespace("Vec3")
        .addCFunction("__call", &lua_NewVec3)
        .addFunction("FromSpherical", &SphericalToCartesian)
        .addFunction("Clamp", &VecClamp<math::vec3>)

        .addProperty("Zero", &StaticVec<math::vec3, float, 0, 0, 0>)
        .addProperty("One", &StaticVec<math::vec3, float, 1, 1, 1>)

        .addProperty("Up", &StaticVec<math::vec3, float, 0, 0, 1>)
        .addProperty("Down", &StaticVec<math::vec3, float, 0, 0, -1>)

        .addProperty("Forward", &StaticVec<math::vec3, float, 1, 0, 0>)
        .addProperty("Backward", &StaticVec<math::vec3, float, -1, 0, 0>)

        .addProperty("Left", &StaticVec<math::vec3, float, 0, 1, 0>)
        .addProperty("Right", &StaticVec<math::vec3, float, 0, -1, 0>)

        .addProperty("X", &StaticVec<math::vec3, float, 1, 0, 0>)
        .addProperty("Y", &StaticVec<math::vec3, float, 0, 1, 0>)
        .addProperty("Z", &StaticVec<math::vec3, float, 0, 0, 1>)
    .endNamespace()
        
    .beginNamespace("Vec2")
        .addCFunction("__call", &lua_NewVec2)
        .addFunction("Clamp", &VecClamp<math::vec2>)

        .addProperty("Zero", &StaticVec<math::vec2, float, 0, 0>)
        .addProperty("One", &StaticVec<math::vec2, float, 1, 1>)

        .addProperty("X", &StaticVec<math::vec2, float, 1, 0>)
        .addProperty("Y", &StaticVec<math::vec2, float, 0, 1>)
    .endNamespace()
    ;
}

//-------------------------------------------------------------------------------------------------

void InitLuaMath(lua_State *lua) {
    DebugLogf(Debug, "Initializing Math module");
    ScriptMathGlobal(lua);
    ScriptMathClasses(lua);
}

} //namespace MoonGlare::Scripts::Modules