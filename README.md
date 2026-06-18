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

---

## 7. Analyse exhaustive en cours (mise à jour incrémentale)

Une revue fichier par fichier de l'intégralité du dépôt est en cours (hors SDK tiers confirmés : STLport, XtreamToolKit, DirectX, Oracle OCI, FreeType, HShield/nProtect, RenderWare, DirectShow, FMOD — ~4750 fichiers / ~1,6M lignes de code propre au jeu à couvrir). Cette section est mise à jour à chaque système entièrement analysé. Le détail complet de chaque système est dans le projet `Archlord_UE5_Project` (`module_notes/`) ; ce qui suit est un résumé.

### ✅ Système couvert intégralement : Siege War / Castle / Archlord

**Découverte clé : le module `AgsmCastle` / `AgpmCastle` (section 1 et 2 ci-dessus) est un stub vide, jamais implémenté.** Toute la logique de château et de siège vit en réalité dans `Modules/AServer/AgsmSiegeWar/` (5884 lignes) et sa contrepartie `Modules/APublic/AgpmSiegeWar/`. À corriger mentalement par rapport aux sections précédentes de ce document.

Points clés extraits (logique métier, pas de code copié) :

- **Machine à états du siège** : `OFF → PROCLAIM_WAR → READY → START → BREAK_A_SEAL → OPEN_EYES → TIME_OVER`, avec embranchement spécial `ARCHLORD_BATTLE → TIME_OVER_ARCHLORD_BATTLE` pour le château de l'Archlord (le "roi" du serveur).
- **Mécanique des "Yeux d'Archon"** : deux yeux à ouvrir séparément ; si le second n'ouvre pas dans le délai imparti, les deux se referment (échec, retour à zéro).
- **Capture du château ("graver le sceau")** : un membre doit rester à portée du trône (1100 unités) pendant une durée fixe ; annulation automatique si le chef de guilde graveur se déconnecte.
- **Château de l'Archlord** : se déroule le 4ème samedi du mois (les sièges normaux sont décalés d'une semaine s'ils tombent cette semaine-là) ; titre déchu si absence du porteur > 5 min pendant la phase de combat, ou en cas de mort n'importe où sur le serveur ; boss gardien "Dikain" (template ID 1366) spawné quand le château est vacant.
- **Réduction de dégâts selon le type d'attaque contre les structures de siège** (table en dur) : distance 30%, mêlée 70%, arme de siège dédiée 100% (tours des Yeux d'Archon : invulnérables aux attaques normales). Classification arme à distance vs mêlée basée sur le type d'arme équipée ; classification des compétences basée sur leur rayon d'action (seuil à 500 unités).
- **Persistance DB** : 4 tables déduites (`CASTLE`, `SIEGE`, `SIEGEAPPLICATION`, `SIEGEOBJECT`), sauvegarde automatique toutes les 60 secondes pendant un siège actif.
- **Annonces de temps restant** : paliers fixes (50/40/30/20/10/9/8/7/6/5/4/3/2/1 minutes puis 50/40/30/20/10 secondes), un seul paquet réseau envoyé par changement de palier.
- **Règles allié/ennemi en zone de siège** : même camp = allié, sauf cas spécial Archlord où deux guildes différentes sont toujours ennemies même si nominalement du même côté.

**Implication architecture UE5** : le système original tourne sur un tick serveur périodique (polling), pas un modèle événementiel pur — à moderniser vers timers/event dispatchers UE5 plutôt que reproduire le polling. Les "statuts spéciaux" de personnage (bitmask) sont de bons candidats pour des Gameplay Tags / composants dédiés côté Gameplay Ability System.

**Reste à couvrir pour clore totalement ce système** : `AgpmSiegeWar` (structures de données et formats de paquets, fichier le plus volumineux restant), `AgsmArchlord` (logique dédiée au titre, référencée massivement mais pas encore lue en détail), `AgsmEventSiegeWarNPC`, et le pendant client (`AgcmSiegeWar`, `AgcmUICastle`, `AgcmUISiegeWar` — optionnel, moins prioritaire pour une migration server-side).

### 🔜 Prochaines vagues (ordre prévu)
AgpmSiegeWar → AgsmArchlord → AgsmCombat/AgsmSkill (formules génériques) → AgsmCharacter/AgpmCharacter → AgsmItem/AgpmItem → AgsmGuild/AgpmGuild → AgsmDropItem(2) → systèmes sociaux/économiques (Party, Trade, Auction, Mail, Buddy, Quest, AI) → AUtility → AClient → Tools/Etc/Patch2/Projects (scan structurel).

