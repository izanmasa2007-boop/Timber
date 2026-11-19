# Timber V2 — Pràctica dels 9 reptes

Joc *Timber!!!* fet amb **C++ i SFML 3**, transformant el codi base `TimberSFML3.cpp` a través de 9 reptes progressius (RA1, RA3, RA5, RA6).

> Assignatura de Programació · CFGS DAM · La Salle Girona

---

## 🎮 Com jugar

| Tecla | Acció |
|-------|-------|
| **Enter** | Començar / reiniciar / pausar |
| **1 / 2** | Triar mode: Fàcil (6 s) / Difícil (3 s) |
| **Fletxa esquerra** | Tallar pel costat esquerre |
| **Fletxa dreta** | Tallar pel costat dret |
| **Esc** | Sortir |

Esquiva les branques, encadena talls per guanyar temps i supera el teu rècord abans de quedar-te sense vides o sense temps.

---

## 🧩 Els 9 reptes

| # | Repte | Què s'ha fet |
|---|-------|--------------|
| 1 | Recursos propis | Càrrega segura amb comprovació i aturada controlada (excepció + missatge per consola) |
| 2 | Més NPC | Abelles i núvols amb `struct NPC` dins d'un `std::vector` |
| 3 | Bosc d'arbres | `struct Tree` en un vector, dibuixat per capes amb un bucle |
| 4 | Vides | 3 vides, `livesText` al HUD i panell fosc semitransparent |
| 5 | Indicador d'FPS | Càlcul amb `dt` i `static_cast<float>` (divisió entera corregida) |
| 6 | Estructura de branques | Migració a un únic `std::vector<Branch>` recorregut amb iteradors |
| 7 | Dos modes de joc | `enum GameMode` + menú amb `switch` i dos temps inicials |
| 8 | Rècord persistent | `record.txt` amb lectura/escriptura i `try/catch` |
| 9 | Enduriment i depuració | Excepció pròpia, `assert`, i `break`/`continue` justificats |

---

## 🛠️ Compilació

- **Visual Studio 2022** (Windows), configuració **Debug · x64**.
- **SFML 3** instal·lada a `C:\SFML64` (includes a `include`, libs a `lib`).
- Llibreries de depuració: `sfml-graphics-d.lib`, `sfml-window-d.lib`, `sfml-system-d.lib`, `sfml-audio-d.lib`.
- Les DLL `*-d-3.dll` han d'estar a la carpeta de l'executable (`x64\Debug`).
- Estàndard: **ISO C++17**.

Obre `Timber.sln`, selecciona **Debug | x64** i prem **F5**.

---

## 📁 Estructura

```
Timber/
├── Timber.sln
├── README.md
├── docs/            → web de GitHub Pages
└── Timber/
    ├── Project.cpp  → codi del joc
    ├── fonts/  graphics/  sound/
    └── record.txt   → rècord desat
```

---

## 👥 Autoria

Treball en parella (pilot / copilot) — assignatura de Programació, CFGS DAM.

- Izan
- (company/a)

Professor: `rblanch8`
