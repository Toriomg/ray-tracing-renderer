¡Absolutamente! Este proyecto es el ejemplo **perfecto** para aplicar Diseño Orientado a Datos (DOD) y la arquitectura ECS. De hecho, el enunciado te guía directamente hacia ello al pedirte dos versiones: `render-aos` (el enfoque clásico, similar a POO) y `render-soa` (el enfoque DOD puro).

Vamos a desglosar cómo diseñarías y construirías este renderizador de trazado de rayos (`ray tracer`) siguiendo los principios de DOD y ECS.

---

### 1. El Cambio de Mentalidad para el Ray Tracer

*   **Visión POO (Array of Structures - AoS):** "Tengo una escena que es una lista de `Objetos`. Cada `Objeto` es una esfera o un cilindro. Para cada rayo, pregunto a cada `Objeto` en la lista: '¿interseccionas con este rayo?'. El objeto que me devuelva la intersección más cercana gana. Luego le pregunto a ese objeto por su `Material` y le pido que calcule el color."
    *   Esto implica clases base (`Objeto`, `Material`), herencia y funciones virtuales (`intersect()`, `scatter()`).

*   **Visión DOD (Structure of Arrays - SoA):** "Tengo un montón de rayos que procesar. Para encontrar sus intersecciones, necesito un conjunto de datos de esferas (centros y radios) y un conjunto de datos de cilindros (centros, ejes y radios). Un `SistemaDeInterseccion` tomará todos los rayos y los probará contra todos los datos de las esferas. Luego, hará lo mismo con los datos de los cilindros. El sistema guardará la intersección más cercana para cada rayo. Después, un `SistemaDeSombreado` tomará los resultados de la intersección y, usando los datos de los materiales, calculará el color final para cada rayo."
    *   Aquí no hay objetos, solo datos y funciones que los transforman.

---

### 2. Diseño de la Arquitectura ECS para el Proyecto

Basándonos en el enunciado, así definiríamos los Componentes, Entidades y Sistemas.

#### **A. Componentes (Los Datos Puros)**

Identifica los datos atómicos que describen todo en tu escena.

**Componentes Geométricos:**

*   `PositionComponent`: Un vector 3D `(x, y, z)` que representa el centro.
*   `RadiusComponent`: Un `float` para el radio.
*   `AxisComponent`: Un vector 3D `(ax, ay, az)` para el eje del cilindro.
*   `MaterialIdComponent`: Un identificador (podría ser un `int` o un `struct` pequeño) que vincula una forma geométrica con su material.

**Componentes de Material:**

Aquí está el truco. Como los materiales tienen datos diferentes, los separamos.

*   `MatteMaterialComponent`:
    *   `reflectance`: Vector 3D (RGB)
*   `MetalMaterialComponent`:
    *   `reflectance`: Vector 3D (RGB)
    *   `diffusion_factor`: `float`
*   `RefractiveMaterialComponent`:
    *   `index_of_refraction`: `float`

**Componentes de la Escena (Singleton):**

Algunos datos son globales a la escena. Puedes tratarlos como componentes que pertenecen a una "entidad de escena" especial.

*   `CameraConfigComponent`: Contiene `position`, `target`, `north_vector`, `fov`, etc.
*   `RenderConfigComponent`: Contiene `image_width`, `aspect_ratio`, `samples_per_pixel`, `max_depth`, etc.

#### **B. Entidades (Los "Objetos")**

Una entidad es solo un ID. En tu caso, las entidades son las esferas y los cilindros de la escena.

*   **Entidad 1 (una esfera):** Tiene `PositionComponent`, `RadiusComponent`, `MaterialIdComponent`.
*   **Entidad 2 (un cilindro):** Tiene `PositionComponent`, `RadiusComponent`, `AxisComponent`, `MaterialIdComponent`.

No necesitas una clase `Esfera` ni `Cilindro`. La *combinación* de componentes define lo que "es" una entidad.

#### **C. Almacenamiento de Datos (La Clave de SoA)**

Para la versión `render-soa`, aquí es donde ocurre la magia. En lugar de un `std::vector<Esfera>`, tendrás:

```cpp
// Datos para todas las ESFERAS
struct SphereData {
    std::vector<Vec3> centers;       // PositionComponent
    std::vector<float> radii;        // RadiusComponent
    std::vector<MaterialId> materials; // MaterialIdComponent
};

// Datos para todos los CILINDROS
struct CylinderData {
    std::vector<Vec3> centers;       // PositionComponent
    std::vector<float> radii;        // RadiusComponent
    std::vector<Vec3> axes;          // AxisComponent
    std::vector<MaterialId> materials; // MaterialIdComponent
};

// Datos para todos los MATERIALES
struct MaterialData {
    // Almacenes separados para cada tipo
    std::vector<MatteMaterial> matte_materials;
    std::vector<MetalMaterial> metal_materials;
    std::vector<RefractiveMaterial> refractive_materials;
    // Un mapa para buscar el ID del material por su nombre al parsear
    std::unordered_map<std::string, MaterialId> name_to_id;
};

// La "Escena" es simplemente un contenedor de todos estos datos
struct Scene {
    SphereData spheres;
    CylinderData cylinders;
    MaterialData materials;
    // ... otros datos globales como la cámara, configuración, etc.
};
```
`MaterialId` podría ser un struct: `struct MaterialId { MaterialType type; size_t index; };`. `type` es un enum (`MATTE`, `METAL`, `REFRACTIVE`) y `index` es la posición en el vector correspondiente (`matte_materials`, `metal_materials`, etc.).

#### **D. Sistemas (La Lógica de Transformación)**

Los sistemas son funciones que operan sobre estos contenedores de datos.

1.  **`ParserSystem`**:
    *   **Input:** Rutas a los archivos de escena y configuración.
    *   **Proceso:** Lee los archivos línea por línea.
        *   Si lee `matte: mat1 ...`, crea un `MatteMaterial`, lo añade a `materialData.matte_materials` y guarda su `MaterialId` en el mapa `name_to_id`.
        *   Si lee `sphere: 0 0 0 0.65 mat1`, busca "mat1" para obtener su `MaterialId`. Luego hace `push_back` del centro, radio y `MaterialId` en los vectores correspondientes de `sphereData`.
    *   **Output:** Una estructura `Scene` completamente poblada con los datos iniciales.

2.  **`IntersectionSystem`**:
    *   **Input:** Un rayo (`Ray`), la `SphereData` y la `CylinderData`.
    *   **Proceso:** Esta es la función más crítica para el rendimiento.
        *   Recibe un rayo y devuelve la información de la intersección más cercana (`HitRecord`).
        *   **Bucle 1 (Esferas):** Itera desde `i = 0` hasta `spheres.centers.size()`. En cada iteración, usa `spheres.centers[i]` y `spheres.radii[i]` para hacer el cálculo de intersección rayo-esfera. Este bucle será **extremadamente rápido** porque los datos de centros y radios están contiguos en memoria. La CPU puede pre-cargar los siguientes datos eficientemente.
        *   **Bucle 2 (Cilindros):** Hace lo mismo, iterando sobre los datos de los cilindros.
    *   **Output:** Un `HitRecord` opcional (un struct que contiene el punto de impacto, la normal, la distancia `t`, y el `MaterialId` del objeto golpeado).

3.  **`ShadingSystem` (o `RayColorSystem`)**:
    *   **Input:** Un rayo (`Ray`), la `Scene` completa y la profundidad de recursión actual.
    *   **Proceso:**
        1.  Llama al `IntersectionSystem` con el rayo y los datos geométricos de la escena.
        2.  **Si no hay intersección:** Calcula y devuelve el color de fondo (usando la dirección del rayo).
        3.  **Si hay intersección:**
            *   Usa el `MaterialId` del `HitRecord` para obtener los datos del material correcto.
            *   Usa un `switch` sobre el `material.type`:
                *   `case MATTE:`: Ejecuta la lógica de reflexión mate. Genera un nuevo rayo.
                *   `case METAL:`: Ejecuta la lógica de reflexión metálica. Genera un nuevo rayo.
                *   `case REFRACTIVE:`: Ejecuta la lógica de refracción. Genera un nuevo rayo.
            *   Llama recursivamente a sí mismo (`ShadingSystem`) con el nuevo rayo y la profundidad decrementada.
            *   Atenúa el color devuelto por la llamada recursiva con la `reflectance` del material actual.
    *   **Output:** Un color (Vector 3D RGB).

### 3. El Bucle Principal (`main` en `render-soa`)

El `main` orquesta la ejecución de los sistemas.

```cpp
// Pseudocódigo para main() en la versión SoA
int main(int argc, char* argv[]) {
    // 1. Parsear argumentos de línea de comandos

    // 2. SISTEMA DE PARSEO
    // Carga toda la configuración y la geometría en la estructura Scene
    Scene scene = ParserSystem::loadScene(sceneFilePath, configFilePath);

    // 3. Inicializar el buffer de la imagen (usando SoA como pide el enunciado)
    ImageBuffer image(scene.config.width, scene.config.height); // Contendrá 3 vectores: R, G, B

    // 4. Bucle Principal de Renderizado (SISTEMAS DE INTERSECCIÓN Y SOMBREADO)
    // Este es el bucle "caliente" del programa
    for (int j = 0; j < scene.config.height; ++j) {
        for (int i = 0; i < scene.config.width; ++i) {
            Color pixel_color(0, 0, 0);
            for (int s = 0; s < scene.config.samples_per_pixel; ++s) {
                // Generar un rayo para esta muestra de este píxel
                Ray r = CameraSystem::getRay(scene.camera, i, j);
                
                // Calcular el color para este rayo
                pixel_color += ShadingSystem::rayColor(r, scene, scene.config.max_depth);
            }
            // Promediar y corregir gamma
            image.writePixel(i, j, processFinalColor(pixel_color, scene.config.samples_per_pixel));
        }
    }

    // 5. SISTEMA DE SALIDA
    // Escribe el buffer de la imagen a un archivo PPM
    OutputSystem::savePPM("output.ppm", image);

    return 0;
}
```

### Comparación Explícita: `render-aos` vs. `render-soa`

*   **`render-aos` (el baseline no optimizado):**
    *   Tendrás algo como `std::vector<std::unique_ptr<Hittable>> world;`.
    *   `Hittable` es una clase base abstracta con `virtual bool hit(...) = 0;`.
    *   `Sphere` y `Cylinder` heredan de `Hittable`.
    *   El bucle de intersección itera sobre este vector, y cada `world[i]->hit(...)` es una llamada a una función virtual. Esto causa saltos en la memoria (para buscar en la vtable) y `cache misses`, ya que los datos de las esferas y cilindros están dispersos.

*   **`render-soa` (la versión DOD optimizada):**
    *   No hay herencia ni funciones virtuales para la geometría.
    *   El bucle de intersección es un `for` súper simple sobre arrays contiguos de `float` y `Vec3`.
    *   El rendimiento será drásticamente superior porque la CPU puede mantener los datos que necesita en sus cachés L1/L2, y el prefetcher puede adivinar y traer los siguientes datos antes de que se necesiten.

Este proyecto te da la oportunidad de medir empíricamente la diferencia de rendimiento entre un diseño orientado a objetos clásico y un diseño orientado a datos. ¡Es una lección fantástica de arquitectura de computadores