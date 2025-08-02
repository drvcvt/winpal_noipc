# Projektdokumentation & Verbesserungs-Backlog

Dieses Dokument dient dazu, den aktuellen Stand des Projekts zu analysieren, potenzielle Probleme, fehlende Funktionen und Verbesserungsmöglichkeiten zu identifizieren.

---

## Analyse-Punkte

### 1. Hotkeys (Tastenkombinationen)
- **Problem:** Die Dokumentation erwähnt `Alt+Char`-Tastenkombinationen, aber es scheint nur ein globaler Hotkey (`Alt+Space`?) zum Öffnen der Palette implementiert zu sein. Die Möglichkeit, spezifische Befehle direkt über `Alt+<Zeichen>` zu starten, fehlt.
- **Verbesserung:** Ein System implementieren, mit dem Benutzer benutzerdefinierte Hotkeys für ihre Lieblingsbefehle festlegen können.

### 2. Befehlshistorie (Execution History)
- **Problem:** Die Befehlshistorie wird zwar während einer Sitzung angezeigt, aber sie wird nicht auf der Festplatte gespeichert. Nach einem Neustart der Anwendung ist die Historie leer.
- **Verbesserung:** Die Historie in einer Datei (z.B. im `AppData`-Verzeichnis des Benutzers) persistent speichern und beim Start wieder laden. Die Anzahl der zu speichernden Einträge sollte konfigurierbar sein.

### 3. Suche & Leistung
- **Problem:**
    - Die allgemeine Suche ist eine einfache Substring-Suche und unterstützt kein "Fuzzy-Matching", was die Auffindbarkeit von Befehlen bei Tippfehlern erschwert.
    - Die Anwendungssuche (`!l ...` oder `launch ...`) und Prozesssuche (`!t ...` oder `terminate ...`) basieren auf statischen, hartcodierten Listen von populären Programmen. Das macht die Suche schnell, aber sehr unvollständig. Der Benutzer kann keine beliebige installierte Anwendung starten.
- **Verbesserung:**
    - Eine "Fuzzy-Search"-Bibliothek integrieren, um die Suchergebnisse relevanter zu machen.
    - Eine asynchrone Suche nach installierten Anwendungen (z.B. aus dem Startmenü) und laufenden Prozessen implementieren. Der `AsyncSearchManager` scheint dafür vorbereitet zu sein, wird aber aktuell nicht voll genutzt. Die Ergebnisse sollten zwischengespeichert werden, um die Leistung zu optimieren.

### 4. Benutzeroberfläche (UI)
- **Problem:** Die UI ist mit GDI+ manuell gezeichnet. Das ist funktional, aber schwer zu warten und zu erweitern. Kleinere grafische Glitches könnten auftreten.
- **Verbesserung (Langfristig):** Eine modernere UI-Bibliothek wie WinUI 3 oder eine leichtgewichtigere Alternative in Betracht ziehen, um die UI-Entwicklung zu vereinfachen und die visuelle Qualität zu verbessern.

### 5. Plugin-System & Befehle
- **Problem:** Es gibt eine Mischung aus einem "neuen" Plugin-System und "alten" Befehlen, die direkt in `main.cpp` registriert werden. Dies deutet auf eine unvollständige Refaktorierung hin.
- **Verbesserung:** Alle Befehle in das neue Plugin-System überführen, um die Codebasis zu vereinheitlichen und die Wartbarkeit zu verbessern.

### 6. Konfiguration
- **Problem:** Viele Einstellungen (z.B. Hotkeys, Farben, Suchverhalten) sind hartcodiert.
- **Verbesserung:** Eine Einstellungsdatei (z.B. `settings.json`) einführen, damit Benutzer die Anwendung an ihre Bedürfnisse anpassen können. Ein Einstellungs-Befehl (`settings` oder `!z settings`) könnte dann ein UI-Fenster zum Bearbeiten dieser Einstellungen öffnen.
