# Memoria técnica de OpenALPR

Punto de entrada para desarrollo y operación. Instantánea revisada sobre `master` en el commit `736ab0e608cf9b20d92f36a873bb1152240daa98`. Las rutas de esta memoria son relativas a la raíz del repositorio. El código y los archivos de configuración son la fuente de verdad; este documento describe las funciones por interfaz y responsabilidad, no enumera cada método privado ni las bibliotecas de terceros.

## Mapa del proyecto

| Ruta | Responsabilidad |
| --- | --- |
| `src/main.cpp` | CLI `alpr`: imágenes, directorios, vídeo, webcam, entrada estándar y flujos HTTP(S). |
| `src/openalpr/alpr.h`, `alpr.cpp`, `alpr_impl.*` | API C++ pública, implementación, reconocimiento y JSON. |
| `src/openalpr/alpr_c.*` | Interfaz C y gestión de memoria para integraciones. |
| `src/openalpr/config*`, `constants.h` | Configuración global, regional y resolución de recursos. |
| `src/openalpr/detection/` | Detección de candidatos, máscara y variantes CPU, CUDA, OpenCL y morfológica. |
| `src/openalpr/textdetection/`, `edges/`, `transformation.*`, `prewarp.*` | Texto, bordes, esquinas y corrección de perspectiva. |
| `src/openalpr/ocr/`, `ocr/segmentation/`, `postprocess/` | Segmentación, Tesseract, candidatos de caracteres y patrones. |
| `pipeline_data.*`, `licenseplatecandidate.*`, `result_aggregator.*` en `src/openalpr/` | Datos intermedios, análisis de candidatos y combinación de resultados. |
| `src/video/`, `src/daemon.cpp`, `src/daemon/`, `src/inc/safequeue.h` | Captura, daemon, colas, procesamiento y envío opcional. |
| `src/statedetection/` | Identificación experimental de estado/región, opcional en compilación. |
| `src/bindings/` | Integraciones C#, Go, Java y Python; ejemplos y scripts en cada carpeta. El README menciona Node.js, pero este árbol no incluye su binding. |
| `src/misc_utilities/` | Etiquetado, preparación de OCR, calibración y benchmarks. |
| `src/tests/` | Pruebas de API, configuración, expresiones y utilidades. |
| `config/`, `runtime_data/` | Plantillas globales y recursos de detección, OCR y patrones por código. |
| `distros/`, `Dockerfile`, `doc/man/alpr.1` | Empaquetado, contenedor y manual de CLI. |

`src/openalpr/simpleini/`, `src/openalpr/support/re2/`, `src/tclap/` y otras dependencias incluidas son código de soporte/terceros; consultar sus archivos antes de modificarlos.

## Flujo y funciones principales

1. `alpr::Alpr` construye `AlprImpl` con país(es), archivo de configuración y directorio de datos opcionales. `Config` carga ajustes y comprueba la configuración regional y el archivo OCR. Se crean detector y OCR por código.
2. `recognize` recibe archivo, imagen codificada o píxeles con regiones de interés (ROI). `recognizeFullDetails` limita las ROI, convierte a escala de grises y aplica `PreWarp`. Admite códigos separados por comas; combina iteraciones de `analysis_count` y selecciona resultados entre países.
3. `analyzeSingleCountry` detecta candidatos dentro de las ROI o las usa directamente con `skip_detection`. `LicensePlateCandidate::recognize` analiza texto, encuentra bordes/esquinas y corrige perspectiva. Los candidatos descartados no siguen al OCR.
4. El OCR segmenta caracteres y usa Tesseract; el postprocesador evalúa combinaciones, confianza y patrones regionales. Opcionalmente se intenta detectar estado/región si se compiló esa función. `ResultAggregator` combina resultados; se devuelven coordenadas, candidatos y tiempos.
5. `src/main.cpp` presenta texto o JSON; `--motion` limita ROI por movimiento. El daemon captura flujos mediante `VideoBuffer`, procesa con hilos, usa Beanstalk para resultados y puede enviarlos por HTTP. Sus opciones están en `config/alprd.conf.defaults` y `config/alprd.conf.user`.

### Interfaces para integrar

* C++ (`src/openalpr/alpr.h`): `Alpr(country, configFile, runtimeDir)`, `isLoaded()`, `recognize(...)`, `setCountry`, `setPrewarp`, `setMask`, `setDetectRegion`, `setTopN`, `setDefaultRegion`, `toJson`, `fromJson`, `getVersion` y `getConfig`. `AlprResults` contiene dimensiones, ROI y placas; cada `AlprPlateResult` incluye país, esquinas, mejor placa y alternativas.
* C (`src/openalpr/alpr_c.h`): `openalpr_init`, `openalpr_is_loaded`, setters equivalentes, `openalpr_recognize_rawimage` / `openalpr_recognize_encodedimage`, `openalpr_free_response_string` y `openalpr_cleanup`. Liberar cada respuesta y la instancia con las funciones indicadas en el encabezado.
* Python: `src/bindings/python/test.py` muestra construcción, `is_loaded`, `recognize_array` y `unload` en `finally`. Java, Go y C# tienen sus ejemplos junto al código. La instalación CMake de Python apunta a `python2.7/dist-packages`; no asumir que instala automáticamente el paquete para Python 3.

## Instalación y primera ejecución

El proyecto es histórico: el README indica Tesseract 3.0.4 y OpenCV 2.4.8+; `Dockerfile` usa Ubuntu 18.04. Las versiones actuales de dependencias y CMake pueden requerir adaptación. Los siguientes pasos reproducen el entorno de compilación documentado, no garantizan compatibilidad con sistemas modernos.

En un entorno Ubuntu compatible, desde la raíz:

```sh
sudo apt-get update
sudo apt-get install -y build-essential cmake git libcurl3-dev libleptonica-dev liblog4cplus-dev libopencv-dev libtesseract-dev
mkdir -p src/build
cd src/build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_SYSCONFDIR=/etc ..
make -j2
sudo make install
./tests/unittests
alpr -c us -j /ruta/a/imagen.jpg
```

El daemon necesita además Beanstalk para su cola; los bindings y herramientas habilitados pueden tener requisitos adicionales. CMake activa por defecto `WITH_DAEMON`, `WITH_TESTS`, `WITH_UTILITIES`, `WITH_BINDING_JAVA`, `WITH_BINDING_PYTHON` y `WITH_BINDING_GO`; `WITH_STATEDETECTION` y `WITH_GPU_DETECTOR` están desactivados. Se pueden desactivar opciones no necesarias con `-D...=OFF`. El instalador copia `runtime_data` a `/usr/share/openalpr/runtime_data` con el prefijo anterior y configura `/etc/openalpr/openalpr.conf`. Para otras plataformas, consultar los enlaces de compilación del [README](../README.md).

Alternativa usando el Dockerfile local:

```sh
docker build -t openalpr-local .
docker run --rm -v "$PWD:/data:ro" openalpr-local -c eu -j /data/imagen.jpg
```

Para una instalación existente, `alpr --help` muestra opciones; `-c` selecciona código, `-p` patrón, `-n` número de candidatos, `--config` configuración, `--debug` diagnóstico y `-j` JSON. Ejemplo: `alpr -c mx -p mx -j /ruta/a/imagen.jpg`. Usar imágenes con autorización y comprobar resultados reales; un comando que termina no demuestra precisión.

### Configuración

La selección del archivo global prioriza argumento explícito, variable `OPENALPR_CONFIG_FILE`, archivo junto al ejecutable y ubicación predeterminada. Se complementa con `share/openalpr/config/openalpr.defaults.conf`; los valores de usuario prevalecen. `runtime_dir` indica dónde buscar datos; la API también puede pasar el directorio explícitamente. Véase `src/openalpr/config.cpp` para precedencia y valores.

`runtime_data/config/<código>.conf` define geometría de placa y caracteres, número de líneas, umbrales, `ocr_language` y opcionalmente `detector_file`. El detector LBP busca `runtime_data/region/<detector_file>` o `<código>.xml`; OCR busca `runtime_data/ocr/tessdata/<ocr_language>.traineddata`. Los patrones se buscan por **código**, no por detector u OCR compartido, en `runtime_data/postprocess/<código>.patterns`. Si faltan patrones, no se heredan automáticamente. Los valores globales incluyen detector, `skip_detection`, máscara, prewarp, umbrales y opciones `debug_*`.

## Países y recursos incluidos

Inventario de archivos versionados, no garantía de exactitud ni de disponibilidad de cada formato de placa. «Patrones» significa que existe el archivo específico. `eu` es un perfil regional, no un país; sufijos distinguen configuraciones.

| Código | Perfil | Detector LBP | OCR | Patrones |
| --- | --- | --- | --- | --- |
| `au` | Australia | `au.xml` | `lau` | sí |
| `auwide` | Australia ancho | `eu.xml` | `lau` | no |
| `br` | Brasil | `br.xml` | `lbr` | sí |
| `br2` | Brasil, dos líneas | `br2.xml` | `lbr` | no |
| `eu` | Europa | `eu.xml` | `leu` | sí |
| `fr` | Francia | `eu.xml` | `lfr` | no |
| `gb` | Reino Unido | `eu.xml` | `lgb` | sí |
| `in` | India | `in.xml` | `lus` | sí |
| `kr` | Corea | `eu.xml` | `lkr` | sí |
| `kr2` | Corea, dos líneas | `kr2.xml` | `lkr` | no |
| `mx` | México | `us.xml` | `lus` | sí |
| `sg` | Singapur | `eu.xml` | `lsg` | sí |
| `us` | Estados Unidos | `us.xml` | `lus` | sí |
| `vn2` | Vietnam, dos líneas | `vn2.xml` | `leu` | no |

Aunque también existen archivos OCR `lin.traineddata` y `lkr2.traineddata`, los perfiles `in` y `kr2` no los seleccionan en esta versión. Los ejemplos de estados para identificación visual están en `runtime_data/keypoints/us/`; la opción CMake `WITH_STATEDETECTION` está desactivada por defecto. `isLoaded()` refleja la carga de configuración, no certifica que el clasificador LBP se haya cargado: revisar también los errores del detector y probar reconocimiento.

En los patrones cada línea tiene `región patrón`; `@` representa letra, `#` número y `?` posición descartable. Se admiten grupos como `[A-Z]`. Ver `runtime_data/postprocess/readme.txt` y `regexrule.cpp`. `-p` recibe una región presente en el archivo del código elegido, no otro código de país.

## Preparación y entrenamiento por país

La instalación es común; cambian la configuración y los modelos de la tabla. Para un país nuevo o mejorar uno existente:

1. Definir formato(s), alfabeto, dimensiones, iluminación y objetivo de precisión. Reunir imágenes autorizadas, positivas y negativas, con etiquetas fiables; separar entrenamiento, validación y prueba por vehículo/captura para evitar fuga de datos. Registrar procedencia, permisos, versiones y particiones sin publicar matrículas sensibles.
2. Crear o ajustar `runtime_data/config/<código>.conf` a partir de un perfil similar; definir geometría, líneas, caracteres, `ocr_language` y, si procede, `detector_file`. Crear `postprocess/<código>.patterns` para regiones conocidas. No modificar un modelo compartido sin evaluar todos los perfiles que lo usan.
3. Para etiquetar imágenes completas, crear previamente directorios de entrada y salida y ejecutar `openalpr-utils-tagplates <código> <imágenes> <etiquetas>`. Es interactivo y usa GUI; genera archivos `.txt` con nombre de imagen, rectángulo y texto. Revisar etiquetas. `openalpr-utils-classifychars <código> <recortes_de_placas> <caracteres>` permite clasificar caracteres interactivamente; espera recortes y nombres con código de región de dos caracteres al inicio.
4. Alternativamente, generar hojas con la tipografía y caracteres de la placa, fotografiarlas en distintas condiciones y usar `src/build/misc_utilities/openalpr-utils-binarizefontsheet --character_file <lista.txt> --out_dir <caracteres> <hoja.jpg>`; el directorio debe existir. Esta herramienta se compila, pero el CMake actual no la instala. Verificar manualmente imágenes y orden de caracteres. Ejecutar `openalpr-utils-prepcharsfortraining <caracteres>` desde un directorio de trabajo vacío: crea `combined.tif` y `combined.box` allí, tomando el primer carácter del nombre de cada imagen como etiqueta.
5. Entrenar el OCR con herramientas y procedimiento compatibles con la versión de Tesseract elegida, usando los TIFF/BOX preparados y validando sus etiquetas; colocar el `.traineddata` resultante en `runtime_data/ocr/tessdata/` con el nombre indicado por `ocr_language`. Entrenar o reutilizar una cascada OpenCV compatible para detección y colocar el XML en `runtime_data/region/`, ajustando `detector_file`. Estos son procesos distintos: entrenar OCR no genera un detector.
6. Probar con `alpr -c <código> -j <imagen>` y con un conjunto reservado. Comparar detección, falsos positivos, texto exacto/top-N y tiempos frente a la línea base; evaluar también otros perfiles que compartan recursos. Documentar versiones, comandos externos, parámetros, métricas y artefactos para poder reproducir o revertir el cambio.

**Límite importante:** este repositorio contiene utilidades de preparación, no los conjuntos de entrenamiento, recetas completas ni comandos versionados para generar desde cero las cascadas y los modelos Tesseract. No es posible reproducir el entrenamiento completo de cada país únicamente con este árbol; seleccionar y documentar herramientas y datos compatibles antes de ejecutar el paso 5. No sustituir modelos de producción sin evaluación independiente.

## Pruebas y diagnóstico

`src/tests/CMakeLists.txt` registra `unittests`; desde `src/build`, ejecutar `./tests/unittests` o `ctest --output-on-failure`. La CI histórica en `.travis.yml` también compila herramientas e instala antes de ejecutar pruebas. Para medir con datos propios, `openalpr-utils-benchmark <código> <speed|segocr|detection|endtoend> <entrada> <salida>`; revisar el código de cada modo para requisitos de datos y GUI. El modo `endtoend` lee imágenes y etiquetas `.txt` en el directorio de entrada. No confundir pruebas unitarias con medición de precisión por país.

Si no carga: revisar mensajes, archivo global, `runtime_dir`, `<código>.conf` y el OCR seleccionado. Si el detector informa error de cascada, revisar el XML efectivo y compatibilidad OpenCV. Si no encuentra placas, comprobar resolución, ROI/máscara, iluminación y parámetros; activar `--debug` o flags `debug_*` según necesidad. Las opciones gráficas requieren entorno con pantalla. Si los patrones no coinciden, comprobar código, región, archivo específico y reglas; `must_match_pattern` puede descartar resultados.

## Proceso para mantener esta memoria

1. En cada cambio, quien lo implementa identifica las secciones afectadas: API/flujo, compilación, configuración, tabla regional, entrenamiento, pruebas o diagnóstico. Actualiza esta memoria y enlaces en el mismo cambio; no incluir secretos ni datos personales.
2. Si cambia un perfil, comparar su `.conf` con los archivos efectivos de `region/`, `ocr/tessdata/` y `postprocess/`, incluidos perfiles que comparten modelos. Registrar limitaciones y resultados de evaluación; no marcar un perfil como validado solo porque existan archivos.
3. El revisor contrasta rutas, comandos y comportamiento con código y CMake, ejecuta las pruebas apropiadas cuando corresponda y comprueba los enlaces internos. Si no puede validar entrenamiento o instalación, deja explícita la limitación.
4. En cambios estructurales o de modelos, actualizar el commit de referencia de esta memoria al commit base revisado y describir en el cambio qué se verificó. Revisar periódicamente el inventario frente al árbol; esta revisión es manual, no hay automatización configurada.

La lista completa de declaraciones públicas y detalles de implementación permanece en los encabezados y archivos enlazados arriba; al añadir o cambiar funciones, mantener ese contrato y su descripción aquí sincronizados.
