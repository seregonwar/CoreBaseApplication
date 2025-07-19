@echo off
REM Build script per CoreBaseApplication
REM Questo script compila il progetto e esegue tutti i test

echo ========================================
echo CoreBaseApplication Build Script
echo ========================================

REM Controlla se CMake è disponibile
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERRORE: CMake non trovato. Installare CMake e aggiungerlo al PATH.
    pause
    exit /b 1
)

REM Crea directory di build se non esiste
if not exist "build" (
    echo Creazione directory build...
    mkdir build
)

REM Entra nella directory di build
cd build

echo.
echo ========================================
echo Configurazione del progetto con CMake
echo ========================================

REM Configura il progetto
cmake .. -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo ERRORE: Configurazione CMake fallita.
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo Compilazione del progetto
echo ========================================

REM Compila il progetto
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo ERRORE: Compilazione fallita.
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo Esecuzione dei test
echo ========================================

REM Esegui i test
ctest --output-on-failure --config Release
if %errorlevel% neq 0 (
    echo ATTENZIONE: Alcuni test sono falliti.
    echo Controllare l'output sopra per i dettagli.
) else (
    echo Tutti i test sono passati con successo!
)

echo.
echo ========================================
echo Build completato
echo ========================================

REM Mostra i file generati
echo.
echo File generati:
dir /b bin\*.exe 2>nul
dir /b lib\*.lib 2>nul

echo.
echo Binari disponibili in: %cd%\bin
echo Librerie disponibili in: %cd%\lib

cd ..

echo.
echo ========================================
echo Compilazione Rust bindings (opzionale)
echo ========================================

if exist "src\core\bindings\rust\Cargo.toml" (
    echo Compilazione binding Rust...
    cd src\core\bindings\rust
    
    REM Controlla se Rust è disponibile
    cargo --version >nul 2>&1
    if %errorlevel% neq 0 (
        echo ATTENZIONE: Rust/Cargo non trovato. Saltando compilazione binding Rust.
    ) else (
        cargo build --release
        if %errorlevel% neq 0 (
            echo ATTENZIONE: Compilazione binding Rust fallita.
        ) else (
            echo Binding Rust compilati con successo!
        )
    )
    
    cd ..\..\..\..\n) else (
    echo Binding Rust non trovati, saltando...
)

echo.
echo ========================================
echo Compilazione Java bindings (opzionale)
echo ========================================

if exist "src\core\bindings\java\ErrorHandler.java" (
    echo Compilazione binding Java...
    
    REM Controlla se Java è disponibile
    javac -version >nul 2>&1
    if %errorlevel% neq 0 (
        echo ATTENZIONE: Java compiler non trovato. Saltando compilazione binding Java.
    ) else (
        if not exist "build\java" mkdir build\java
        
        REM Compila i file Java
        javac -d build\java src\core\bindings\java\*.java
        if %errorlevel% neq 0 (
            echo ATTENZIONE: Compilazione binding Java fallita.
        ) else (
            echo Binding Java compilati con successo!
            
            REM Crea JAR
            cd build\java
            jar cf ..\corebase-java-bindings.jar com\corebase\*.class
            cd ..\..
            echo JAR creato: build\corebase-java-bindings.jar
        )
    )
) else (
    echo Binding Java non trovati, saltando...
)

echo.
echo ========================================
echo Build script completato!
echo ========================================
echo.
echo Per eseguire i test manualmente:
echo   cd build
echo   ctest --verbose
echo.
echo Per eseguire l'applicazione di esempio:
echo   build\bin\cba_gui.exe
echo.
echo Per maggiori informazioni, consultare README.md
echo.

pause