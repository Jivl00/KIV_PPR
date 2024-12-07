# KIV/PPR - Paralelní programování

## Analýza akcelerometrických dat pomocí paralelního a vektorizovaného zpracování (CPU i GPU)

## Popis projektu
Tato semestrální práce se zaměřuje na výpočet dvou statistických ukazatelů – **koeficientu variace** a **mediánu absolutní odchylky (MAD)** – z akcelerometrických dat získaných ze souborů ve formátu `ACC_*.csv`. Výpočty jsou realizovány pomocí různých technik paralelního programování a vektorizace s cílem analyzovat výkonnost jednotlivých přístupů.

### Implementované varianty výpočtů:
- Sekvenční (sériový) výpočet
- Vektorizovaný výpočet (AVX2)
- Paralelní výpočet (více vláken)
- Paralelní a zároveň vektorizovaný výpočet
- Výpočet na GPU (OpenCL)

Každá varianta je testována na různých velikostech vstupních dat a spouštěna 10× za účelem zjištění mediánu výpočetního času.

## Požadavky
- CMake ≥ 3.21
- MSVC 2022 (64bit)
- C++17 nebo novější
- Volitelně: OpenCL SDK (pro GPU variantu)

## Kompilace
Použij CMake a Visual Studio nebo příkazovou řádku:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Pro zapnutí single precision varianty:

1. Uprav soubor `CMakeLists.txt`:

```cmake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /arch:AVX2 /D_FLOAT")
```

Nebo přidej při volání CMake:

```bash
cmake -S . -B build -DCMAKE_CXX_FLAGS="-D_FLOAT"
```

## Spuštění programu

### Povinný argument

* `--input <cesta>` – cesta k souboru nebo adresáři se soubory ACC\*.csv

### Volitelné argumenty

* `--output <cesta>` – výstupní adresář (výchozí `results`)
* `--repetitions <n>` – počet opakování každého výpočtu (výchozí 1)
* `--num_partitions <n>` – počet vláken/paralelních bloků (výchozí 1)
* `--gpu` – aktivuje GPU variantu (OpenCL)
* `--parallel` – spustí paralelní variantu na CPU
* `--vectorized` – zapne AVX2 vektorizaci
* `--all_variants` – spustí všechny varianty výpočtu najednou

### Příklady spuštění

Spuštění všech variant:

```bash
program.exe --input data/ --output results --repetitions 10 --all_variants
```

Spuštění paralelní a vektorizované varianty:

```bash
program.exe --input data/ACC_001.csv --parallel --vectorized
```

## Výstup

Program vygeneruje:

* Statistické hodnoty pro každý rozsah dat
* Mediány výpočetních časů
* 3 grafy ve formátu SVG:

  1. Výpočetní časy jednotlivých variant
  2. Koeficient variace
  3. Medián absolutní odchylky

## Poznámky

* Pro správné fungování je nutné použít 64bitový překladač.
* Při běhu na GPU se doporučuje systém s podporou OpenCL a dostatečnou VRAM.
