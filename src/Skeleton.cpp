//=============================================================================================
// Computer Graphics Sample Program: Ray-tracing-let
//=============================================================================================
#include "framework.h"

enum MaterialType { ROUGH, REFLECTIVE };

struct Material {
    vec3 ka, kd, ks;
    float shininess;
    vec3 F0;
    MaterialType type;

    Material(MaterialType t) { type = t; }
};

struct RoughMaterial : Material {
    RoughMaterial(vec3 _kd, vec3 _ks, float _shininess) : Material(ROUGH) {
        ka = _kd * M_PI;
        kd = _kd;
        ks = _ks;
        shininess = _shininess;
    }
};

struct ReflectiveMaterial : Material {
    ReflectiveMaterial(vec3 n, vec3 kappa) : Material(REFLECTIVE) {
        vec3 one(1,1,1);
        vec3 num = (n - one) * (n - one) + kappa * kappa;
        vec3 denon = (n + one) * (n + one) + kappa * kappa;
        F0 = vec3(
            num.x / denon.x,
            num.y / denon.y,
            num.z / denon.z
        );
    }    
};

struct Hit {
    float t;
    vec3 position, normal;
    Material * material;
    Hit() { t = -1; }
};

struct Ray {
    vec3 start, dir;
    Ray(vec3 _start, vec3 _dir) { start = _start; dir = normalize(_dir); }
};

class Intersectable {
protected:
    Material * material;
public:
    virtual Hit intersect(const Ray& ray) = 0;
};

struct Ellipsoid : public Intersectable {
    vec3 center;
    float A, B, C, cutY;

    Ellipsoid(const vec3& _center, float _A, float _B, float _C, Material* _material, float _cutY = 1000) {
        center = _center;
        A = _A;
        B = _B;
        C = _C;
        cutY = _cutY;
        material = _material;
    }

    Hit intersect(const Ray& ray) {
        Hit hit;
        vec3 dist = ray.start - center;

        float a = powf(ray.dir.x, 2) / powf(A, 2)
            + powf(ray.dir.y, 2) / powf(B, 2)
            + powf(ray.dir.z, 2) / powf(C, 2);
        
        float b = 2.0f * ((dist.x * ray.dir.x) / powf(A, 2)
            + (dist.y * ray.dir.y) / powf(B, 2)
            + (dist.z * ray.dir.z) / powf(C, 2));
        
        float c = powf(dist.x, 2) / powf(A, 2)
            + powf(dist.y, 2) / powf(B, 2)
            + powf(dist.z, 2) / powf(C, 2)
            - 1.0f;

        float discr = powf(b, 2) - 4.0f * a * c;
        if (discr < 0.0f) return hit;
        
        float sqrt_discr = sqrt(discr);
        
        float t1 = (-b + sqrt_discr) / 2.0f / a;
        float t2 = (-b - sqrt_discr) / 2.0f / a;
        
        if (t1 <= 0) return hit;
        
        hit.t = (t2 > 0) ? t2 : t1;
        hit.position = ray.start + ray.dir * hit.t;

        if(hit.position.y > cutY) return Hit();

        hit.normal = normalize(
            vec3(
                (hit.position - center).x / (A * A),
                (hit.position - center).y / (B * B),
                (hit.position - center).z / (C * C)
            )
        ); 
        hit.material = material;
        return hit;
    }
};

class Camera {
    vec3 eye, lookat, right, up;
public:
    void set(vec3 _eye, vec3 _lookat, vec3 vup, float fov) {
        eye = _eye;
        lookat = _lookat;
        vec3 w = eye - lookat;
        float focus = length(w);
        right = normalize(cross(vup, w)) * focus * tanf(fov / 2);
        up = normalize(cross(w, right)) * focus * tanf(fov / 2);
    }
    Ray getRay(int X, int Y) {
        vec3 dir = lookat + right * (2.0f * (X + 0.5f) / windowWidth - 1) + up * (2.0f * (Y + 0.5f) / windowHeight - 1) - eye;
        return Ray(eye, dir);
    }
};

struct Light {
    vec3 direction;
    vec3 Le;
    Light(vec3 _direction, vec3 _Le) {
        direction = normalize(_direction);
        Le = _Le;
    }
};

float rnd() { return (float)rand() / RAND_MAX; }

const float epsilon = 0.0001f;

class Scene {
    std::vector<Intersectable *> objects;
    std::vector<Light *> lights;
    Camera camera;
    vec3 La;
public:
    void build() {
        vec3 eye = vec3(0, -0.4f, 2.5f), vup = vec3(0, 1, 0.1f), lookat = vec3(0, 0.4f, 0);
        float fov = 45 * M_PI / 180;
        camera.set(eye, lookat, vup, fov);

        La = vec3(0.4f, 0.4f, 0.4f);
        vec3 lightDirection(1, 8, 1), Le(2, 2, 2);
        lights.push_back(new Light(lightDirection, Le));

        vec3 kd1(0.05f, 0.6f, 0.05f), ks1(1, 1, 1);
        Material * material1 = new RoughMaterial(kd1, ks1, 50);
        objects.push_back(new Ellipsoid(vec3(-0.35f, -0.35f, 0.15f), 0.3f, 0.15f, 0.3f, material1));

        vec3 kd2(0.7f, 0.2f, 0.2f), ks2(1, 1, 1);
        Material * material2 = new RoughMaterial(kd2, ks2, 50); 
        objects.push_back(new Ellipsoid(vec3(-0.1f, -0.2f, -0.35f), 0.1f, 0.3f, 0.2f, material2));

        vec3 n(0.17f, 0.35f, 1.5f), kappa(3.1f, 2.7f, 1.9f);
        Material * reflectiveMaterial = new ReflectiveMaterial(n, kappa);
        objects.push_back(new Ellipsoid(vec3(0.4f, 0.05f, -0.2f), 0.2f, 0.5f, 0.3f, reflectiveMaterial));

        vec3 kd3(0.8f, 0.6f, 0.2f), ks3(1, 1, 1);
        Material * material3 = new RoughMaterial(kd3, ks3, 50); 
        objects.push_back(new Ellipsoid(vec3(0.0f, 0.45f, 0.0f), 4.9f, 1.0f, 4.9f, material3, 1.44f));
    }

    void render(std::vector<vec4>& image) {
        for (int Y = 0; Y < windowHeight; Y++) {
            #pragma omp parallel for
            for (int X = 0; X < windowWidth; X++) {
                vec3 color = trace(camera.getRay(X, Y));
                image[Y * windowWidth + X] = vec4(color.x, color.y, color.z, 1);
            }
        }
    }

    Hit firstIntersect(Ray ray) {
        Hit bestHit;
        for (Intersectable * object : objects) {
            Hit hit = object->intersect(ray); //  hit.t < 0 if no intersection
            if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t))  bestHit = hit;
        }
        if (dot(ray.dir, bestHit.normal) > 0) bestHit.normal = bestHit.normal * (-1);
        return bestHit;
    }

    bool shadowIntersect(Ray ray) {    // for directional lights
        for (Intersectable * object : objects) if (object->intersect(ray).t > 0) return true;
        return false;
    }

    vec3 trace(Ray ray, int depth = 0) {
        if(depth > 5) return La;
        Hit hit = firstIntersect(ray);
        if (hit.t < 0) return La;
        
        vec3 outRadiance(0, 0, 0);

        if (hit.material->type == ROUGH) {
            outRadiance = hit.material->ka * La;
            for (Light * light : lights) {
                Ray shadowRay(hit.position + hit.normal * epsilon, light->direction);
                float cosTheta = dot(hit.normal, light->direction);
                if (cosTheta > 0 && !shadowIntersect(shadowRay)) {    // shadow computation
                    outRadiance = outRadiance + light->Le * hit.material->kd * cosTheta;
                    vec3 halfway = normalize(-ray.dir + light->direction);
                    float cosDelta = dot(hit.normal, halfway);
                    if (cosDelta > 0) outRadiance = outRadiance + light->Le * hit.material->ks * powf(cosDelta, hit.material->shininess);
                }
            }
        }
        if (hit.material->type == REFLECTIVE) {
            vec3 reflectedDir = ray.dir - hit.normal * dot(hit.normal, ray.dir) * 2.0f;
            float cosa = -dot(ray.dir, hit.normal);
            vec3 one(1, 1, 1);
            vec3 F = hit.material->F0 + (one - hit.material->F0) * powf(1-cosa, 5);
            outRadiance = outRadiance + trace(Ray(hit.position + hit.normal * epsilon, reflectedDir), depth+1) * F;   
        }

        return outRadiance;
    }
};

GPUProgram gpuProgram;
Scene scene;

// vertex shader in GLSL
const char *vertexSource = R"(
    #version 330
    precision highp float;

    layout(location = 0) in vec2 cVertexPosition;    // Attrib Array 0
    out vec2 texcoord;

    void main() {
        texcoord = (cVertexPosition + vec2(1, 1))/2;                            // -1,1 to 0,1
        gl_Position = vec4(cVertexPosition.x, cVertexPosition.y, 0, 1);         // transform to clipping space
    }
)";

// fragment shader in GLSL
const char *fragmentSource = R"(
    #version 330
    precision highp float;

    uniform sampler2D textureUnit;
    in  vec2 texcoord;            // interpolated texture coordinates
    out vec4 fragmentColor;        // output that goes to the raster memory as told by glBindFragDataLocation

    void main() {
        fragmentColor = texture(textureUnit, texcoord); 
    }
)";

class FullScreenTexturedQuad {
    unsigned int vao;    // vertex array object id and texture id
    Texture texture;
public:
    FullScreenTexturedQuad(int windowWidth, int windowHeight, std::vector<vec4>& image)
        : texture(windowWidth, windowHeight, image) 
    {
        glGenVertexArrays(1, &vao);    // create 1 vertex array object
        glBindVertexArray(vao);        // make it active

        unsigned int vbo;        // vertex buffer objects
        glGenBuffers(1, &vbo);    // Generate 1 vertex buffer objects

        // vertex coordinates: vbo0 -> Attrib Array 0 -> vertexPosition of the vertex shader
        glBindBuffer(GL_ARRAY_BUFFER, vbo); // make it active, it is an array
        float vertexCoords[] = { -1, -1,  1, -1,  1, 1,  -1, 1 };    // two triangles forming a quad
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_STATIC_DRAW);       // copy to that part of the memory which is not modified 
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);     // stride and offset: it is tightly packed
    }

    void Draw() {
        glBindVertexArray(vao);    // make the vao and its vbos active playing the role of the data source
        gpuProgram.setUniform(texture, "textureUnit");
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);    // draw two triangles forming a quad
    }
};

FullScreenTexturedQuad * fullScreenTexturedQuad;

void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);
    scene.build();
    std::vector<vec4> image(windowWidth * windowHeight);
    long timeStart = glutGet(GLUT_ELAPSED_TIME);
    scene.render(image);
    long timeEnd = glutGet(GLUT_ELAPSED_TIME);
    printf("Rendering time: %d milliseconds\n", (timeEnd - timeStart));
    fullScreenTexturedQuad = new FullScreenTexturedQuad(windowWidth, windowHeight, image);
    gpuProgram.create(vertexSource, fragmentSource, "fragmentColor");
}

void onDisplay() {
    fullScreenTexturedQuad->Draw();
    glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY) {}
void onKeyboardUp(unsigned char key, int pX, int pY) {}
void onMouse(int button, int state, int pX, int pY) {}
void onMouseMotion(int pX, int pY) {}
void onIdle() {}
