#!/bin/bash

# Build script per CoreBaseApplication
# Questo script compila il progetto e esegue tutti i test

set -e  # Exit on any error

echo "========================================"
echo "CoreBaseApplication Build Script"
echo "========================================"

# Controlla se CMake è disponibile
if ! command -v cmake &> /dev/null; then
    echo "ERRORE: CMake non trovato. Installare CMake."
    exit 1
fi

# Controlla se make è disponibile
if ! command -v make &> /dev/null; then
    echo "ERRORE: make non trovato. Installare build-essential."
    exit 1
fi

# Crea directory di build se non esiste
if [ ! -d "build" ]; then
    echo "Creazione directory build..."
    mkdir -p build
fi

# Entra nella directory di build
cd build

echo ""
echo "========================================"
echo "Configurazione del progetto con CMake"
echo "========================================"

# Configura il progetto
cmake .. -DCMAKE_BUILD_TYPE=Release

echo ""
echo "========================================"
echo "Compilazione del progetto"
echo "========================================"

# Determina il numero di core per la compilazione parallela
if command -v nproc &> /dev/null; then
    CORES=$(nproc)
elif command -v sysctl &> /dev/null; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=4
fi

echo "Compilazione con $CORES core..."

# Compila il progetto
make -j$CORES

echo ""
echo "========================================"
echo "Esecuzione dei test"
echo "========================================"

# Esegui i test
if ctest --output-on-failure; then
    echo "Tutti i test sono passati con successo!"
else
    echo "ATTENZIONE: Alcuni test sono falliti."
    echo "Controllare l'output sopra per i dettagli."
fi

echo ""
echo "========================================"
echo "Build completato"
echo "========================================"

# Mostra i file generati
echo ""
echo "File generati:"
ls -la bin/ 2>/dev/null || echo "Nessun binario trovato"
ls -la lib/ 2>/dev/null || echo "Nessuna libreria trovata"

echo ""
echo "Binari disponibili in: $(pwd)/bin"
echo "Librerie disponibili in: $(pwd)/lib"

cd ..

echo ""
echo "========================================"
echo "Compilazione Rust bindings (opzionale)"
echo "========================================"

if [ -f "src/core/bindings/rust/Cargo.toml" ]; then
    echo "Compilazione binding Rust..."
    cd src/core/bindings/rust
    
    # Controlla se Rust è disponibile
    if command -v cargo &> /dev/null; then
        if cargo build --release; then
            echo "Binding Rust compilati con successo!"
        else
            echo "ATTENZIONE: Compilazione binding Rust fallita."
        fi
    else
        echo "ATTENZIONE: Rust/Cargo non trovato. Saltando compilazione binding Rust."
    fi
    
    cd ../../../..
else
    echo "Binding Rust non trovati, saltando..."
fi

echo ""
echo "========================================"
echo "Compilazione Java bindings (opzionale)"
echo "========================================"

if [ -f "src/core/bindings/java/ErrorHandler.java" ]; then
    echo "Compilazione binding Java..."
    
    # Controlla se Java è disponibile
    if command -v javac &> /dev/null; then
        mkdir -p build/java
        
        # Compila i file Java
        if javac -d build/java src/core/bindings/java/*.java; then
            echo "Binding Java compilati con successo!"
            
            # Crea JAR
            cd build/java
            if command -v jar &> /dev/null; then
                jar cf ../corebase-java-bindings.jar com/corebase/*.class
                cd ../..
                echo "JAR creato: build/corebase-java-bindings.jar"
            else
                cd ../..
                echo "ATTENZIONE: jar command non trovato, JAR non creato."
            fi
        else
            echo "ATTENZIONE: Compilazione binding Java fallita."
        fi
    else
        echo "ATTENZIONE: Java compiler non trovato. Saltando compilazione binding Java."
    fi
else
    echo "Binding Java non trovati, saltando..."
fi

echo ""
echo "========================================"
echo "Installazione dipendenze Python (opzionale)"
echo "========================================"

if [ -f "src/core/bindings/python/requirements.txt" ]; then
    echo "Installazione dipendenze Python..."
    
    if command -v python3 &> /dev/null; then
        if command -v pip3 &> /dev/null; then
            pip3 install -r src/core/bindings/python/requirements.txt
            echo "Dipendenze Python installate!"
        else
            echo "ATTENZIONE: pip3 non trovato."
        fi
    else
        echo "ATTENZIONE: python3 non trovato."
    fi
else
    echo "File requirements.txt non trovato, saltando..."
fi

echo ""
echo "========================================"
echo "Build script completato!"
echo "========================================"
echo ""
echo "Per eseguire i test manualmente:"
echo "  cd build"
echo "  ctest --verbose"
echo ""
echo "Per eseguire l'applicazione di esempio:"
echo "  ./build/bin/cba_gui"
echo ""
echo "Per maggiori informazioni, consultare README.md"
echo ""