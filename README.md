# AL_Emu_Latest — Notes de référence (usage interne, lecture seule)

> ⚠️ **Statut** : ce dépôt contient le code source quasi-complet du client et du serveur originaux d'Archlord (C++ / Visual Studio, ~2005-2006). Il n'y a ni licence ni attribution claire (commit unique "Initial Push CMB"). À la différence d'[ALEmu](https://github.com/CoolManBob/ALEmu) (GPL-3.0), **aucun usage du code n'est couvert légalement ici**.
>
> **Règle d'usage pour le projet Archlord_UE5_Project : lecture seule, à des fins de compréhension de la logique métier et des structures de données originales. Aucun code de ce repo n'est copié ni porté tel quel dans le projet Unreal Engine.**

Ce document recense uniquement les dossiers utiles à la **migration de la logique server-side** vers notre architecture (PostgreSQL + Redis + TypeScript Login Server + Unreal Engine Game Server). Le reste du repo (client de rendu, outils d'édition de maps/modèles, DirectShow, FMOD, libs Oracle/OCI...) n'est pas pertinent pour nos besoins et n'est pas listé ici.

---

## 1. Architecture serveur originale (référence)

Le serveur original est éclaté en plusieurs process séparés — utile pour comprendre la séparation des responsabilités que nous reproduisons avec notre architecture à 3 machines (Dev / Login Server / Game Server) :

| Dossier | Rôle d'origine | Équivalent dans notre projet |
|---|---|---|
| `Server/AlefServer/` | Serveur de jeu principal (process hôte) | Game Server (UE5) |
| `Server/AgsEngine/` | Moteur du process de jeu, boucle principale | Game Server (UE5 / Mass Entity) |
| `Server/AlefLoginServer/` | Authentification, comptes | Login Server (TypeScript) |
| `Server/AlefRelayServer/`, `Server/AlefRelayServer2/` | Relais inter-serveurs / inter-channel | À évaluer (notre archi est seamless, sans channel) |
| `Server/AlefChattingServer/` | Chat global/serveur dédié | Game Server ou microservice chat |
| `Server/AlefAuctionServer/` | Hôtel des ventes | Game Server (module dédié) |
| `Server/AlefRecruitServer/` | Recrutement de guilde/raid | Game Server (module dédié) |
| `Server/AlefMigrateDB/` | Migration/maintenance de base de données | Référence schéma PostgreSQL |

---

## 2. Logique de gameplay côté serveur — `Modules/AServer/`

C'est le **cœur fonctionnel** à analyser pour notre migration. Chaque sous-dossier `Agsm*` est un module isolé correspondant à un système de jeu. Liste des modules essentiels, classés par priorité pour notre roadmap :

### Priorité haute — systèmes fondamentaux
- `AgsmCharacter/` — création, stats, progression de personnage
- `AgsmCombat/` — résolution des combats, dégâts, formules
- `AgsmSkill/` — compétences, arbres de compétences, cooldowns
- `AgsmItem/` — items, inventaire, équipement
- `AgsmDropItem/`, `AgsmDropItem2/` — tables de loot et timers de loot
- `AgsmDeath/` — mort, respawn, pénalités
- `AgsmZoning/`, `AgsmMap/` — découpage du monde, transitions de zone (monde seamless)
- `AgsmAOIFilter/` — Area of Interest (pertinent pour la visibilité à 1000+ joueurs en Castle Wars)
- `AgsmObject/` — gestion des entités/objets du monde

### Priorité haute — PvP, guildes, monde
- `AgsmPvP/` — règles d'engagement PvP
- `AgsmGuild/`, `AgsmGuildWarehouse/` — guildes et coffres de guilde
- `AgsmCastle/`, `AgsmSiegeWar/` — Castle Wars / sièges (système signature du jeu)
- `AgsmBattleGround/` — champs de bataille instanciés (si applicable à notre design seamless)
- `AgsmShrine/`, `AgsmShrineBattle/` — sanctuaires et combats associés

### Priorité moyenne — systèmes sociaux et économiques
- `AgsmParty/` — groupes
- `AgsmTrade/`, `AgsmPrivateTrade/` — échanges entre joueurs
- `AgsmAuction/`, `AgsmAuctionServer/`, `AgsmAuctionServerDB/` — hôtel des ventes
- `AgsmBuddy/` — liste d'amis
- `AgsmMailBox/` — courrier en jeu
- `AgsmRide/` — montures
- `AgsmSummons/` — invocations/pets
- `AgsmQuest/`, `AgsmEventQuest/` — quêtes
- `AgsmNpcManager/`, `AgsmAI/`, `AgsmAI2/` — PNJ et IA

### Priorité moyenne — comptes, connexion, persistance
- `AgsmAccountManager/` — gestion de comptes (référence pour le Login Server TS)
- `AgsmLoginServer/`, `AgsmLoginClient/`, `AgsmLoginDB/` — flux d'authentification
- `AgsmCharManager/`, `AgsmUsedCharDataPool/` — chargement/sauvegarde de personnage
- `AgsmDatabasePool/`, `AgsmDBPool/`, `AgsmDatabaseConfig/` — pooling DB (référence conceptuelle pour notre PgBouncer)
- `AgsmMakeSQL/` — génération de requêtes SQL, utile pour déduire le **schéma de données d'origine**

### Priorité basse / à évaluer au cas par cas
- `AgsmChatting/`, `AgsmAreaChatting/`, `AgsmGlobalChatting/` — chat (selon besoin)
- `AgsmTitle/`, `AgsmFactors/` — titres et stats dérivées
- `AgsmGamble/`, `AgsmCashMall/` — systèmes monétisation d'origine (non repris — Stripe gère notre côté paiement)
- `AgsmBilling*` (Billing, BillingChina, BillingJapan, BillingGlobal) — **non pertinent**, systèmes de paiement région-spécifiques d'origine

---

## 3. Structures de données partagées — `Modules/APublic/`

Contient les définitions de structures/protocoles partagées entre client et serveur (`Agpm*`), miroir de `Modules/AServer/`. Utile en complément pour comprendre le **format des paquets réseau et des structures de données** échangées (mise en parallèle avec les structures packet d'ALEmu déjà utilisées comme référence).

Modules à consulter en priorité, en miroir de la section 2 :
- `AgpmCharacter/`, `AgpmCombat/`, `AgpmItem/`, `AgpmDropItem/`, `AgpmDropItem2/`, `AgpmCastle/`, `AgpmGuild/`, `AgpmGuildWarehouse/`, `AgpmAuction/`, `AgpmAuctionCategory/`, `AgpmBuddy/`, `AgpmConfig/`

---

## 4. Utilitaires bas niveau potentiellement utiles — `Modules/AUtility/`

La majorité de ces libs sont des utilitaires C++ génériques (non portables tels quels), mais certains éclairent des mécaniques serveur précises :

- `AuDatabase/`, `AuDatabase2/` — couche d'accès DB d'origine (référence pour comprendre le schéma)
- `AuPacket/` — structure bas niveau des paquets réseau
- `AuGenerateID/` — génération d'identifiants uniques (personnages, items, etc.)
- `AuRandomNumberGenerator/` — RNG utilisé pour les calculs de loot/combat (utile pour reproduire fidèlement les probabilités)
- `AuTickCounter/`, `AuTimeStamp/` — gestion du temps serveur (timers de loot, respawn, etc.)
- `AuScriptEngine/`, `AuLua/` — scripting serveur (mise en parallèle avec notre usage d'UnLua)

---

## 5. Ce qui n'est PAS pertinent pour la migration server-side

Pour clarifier le périmètre et éviter toute confusion future :
- `Client/`, `Etc/Acu*`, `RW/` — moteur de rendu, RenderWare, UI client (notre rendu est natif UE5)
- `Tools/` (MapTool, ModelTool, EffTool, RegionTool, ShopClient, UITool, Casper) — outils d'édition d'assets d'origine
- `Patch2/` — système de patch/mise à jour propriétaire d'origine
- `SoundLibrary/` (FMOD) — audio client
- `Lib/`, `Include/` (hors structures de gameplay) — dépendances bas niveau Windows/DirectX/Oracle
- `DynCode/` — protections/anti-tamper propriétaires
- Tous les modules `AgsmBilling*`, `AgsmCashMall`, `AgsmGamble` — systèmes de paiement/région d'origine, hors périmètre (Stripe géré côté Login Server)

---

## 6. Méthodologie d'usage recommandée

1. Pour chaque système du `docs/modules.md` (projet `Archlord_UE5_Project`), identifier le module `Agsm*` / `Agpm*` correspondant ci-dessus.
2. Lire la logique métier (formules, conditions, états) — **jamais copier le code**.
3. Croiser avec les structures packet/SQL d'ALEmu déjà documentées pour valider la cohérence des deux sources.
4. Documenter la mécanique comprise dans le `docs/gameplay_*.md` correspondant du projet UE5, en langage/pseudocode neutre.
5. Faire valider par Faf toute mécanique ambiguë avant implémentation (priorité à la précision sur la vitesse).
