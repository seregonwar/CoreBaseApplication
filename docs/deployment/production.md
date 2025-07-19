# Production Deployment Guide

This guide covers deploying CoreBaseApplication (CBA) in production environments, including containerization, orchestration, monitoring, and best practices for scalable, reliable deployments.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Build Configuration](#build-configuration)
- [Containerization](#containerization)
- [Kubernetes Deployment](#kubernetes-deployment)
- [Docker Compose](#docker-compose)
- [Load Balancing](#load-balancing)
- [Monitoring and Observability](#monitoring-and-observability)
- [Security Hardening](#security-hardening)
- [Performance Tuning](#performance-tuning)
- [Backup and Recovery](#backup-and-recovery)
- [CI/CD Pipeline](#cicd-pipeline)
- [Troubleshooting](#troubleshooting)

## Prerequisites

### System Requirements

**Minimum Requirements:**
- CPU: 2 cores
- RAM: 4GB
- Storage: 20GB SSD
- Network: 1Gbps

**Recommended for Production:**
- CPU: 4+ cores
- RAM: 8GB+
- Storage: 50GB+ SSD with RAID
- Network: 10Gbps
- Load balancer
- Monitoring system

### Software Dependencies

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    wget \
    unzip \
    libssl-dev \
    libcurl4-openssl-dev \
    libjsoncpp-dev \
    libboost-all-dev

# CentOS/RHEL
sudo yum groupinstall -y "Development Tools"
sudo yum install -y \
    cmake3 \
    git \
    curl \
    wget \
    unzip \
    openssl-devel \
    libcurl-devel \
    jsoncpp-devel \
    boost-devel
```

## Build Configuration

### Production Build

```bash
#!/bin/bash
# build-production.sh

set -e

# Configuration
BUILD_TYPE="Release"
INSTALL_PREFIX="/opt/cba"
CONFIG_DIR="/etc/cba"
LOG_DIR="/var/log/cba"
DATA_DIR="/var/lib/cba"

# Create directories
sudo mkdir -p $INSTALL_PREFIX $CONFIG_DIR $LOG_DIR $DATA_DIR
sudo chown -R cba:cba $INSTALL_PREFIX $CONFIG_DIR $LOG_DIR $DATA_DIR

# Build
mkdir -p build-production
cd build-production

cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
    -DCBA_CONFIG_DIR=$CONFIG_DIR \
    -DCBA_LOG_DIR=$LOG_DIR \
    -DCBA_DATA_DIR=$DATA_DIR \
    -DCBA_ENABLE_TESTS=OFF \
    -DCBA_ENABLE_BENCHMARKS=OFF \
    -DCBA_ENABLE_STATIC_ANALYSIS=OFF \
    -DCBA_ENABLE_SECURITY_HARDENING=ON \
    -DCBA_ENABLE_OPTIMIZATION=ON

make -j$(nproc)
sudo make install

# Set permissions
sudo chmod +x $INSTALL_PREFIX/bin/*
sudo chmod 644 $CONFIG_DIR/*
sudo chmod 755 $LOG_DIR $DATA_DIR

echo "Production build completed successfully"
```

### CMake Production Configuration

```cmake
# CMakeLists.txt - Production settings
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Optimization flags
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
    
    # Security hardening
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE")
    
    # Link-time optimization
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    
    # Strip debug symbols
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
endif()

# Production-specific options
option(CBA_ENABLE_SECURITY_HARDENING "Enable security hardening" ON)
option(CBA_ENABLE_OPTIMIZATION "Enable aggressive optimization" ON)
option(CBA_ENABLE_PROFILING "Enable profiling support" OFF)
```

## Containerization

### Multi-stage Dockerfile

```dockerfile
# Dockerfile
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libcurl4-openssl-dev \
    libjsoncpp-dev \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
WORKDIR /src
COPY . .

# Build application
RUN mkdir build && cd build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCBA_ENABLE_TESTS=OFF \
        -DCBA_ENABLE_SECURITY_HARDENING=ON && \
    make -j$(nproc) && \
    make install DESTDIR=/app

# Production image
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libssl3 \
    libcurl4 \
    libjsoncpp25 \
    libboost-system1.74.0 \
    libboost-filesystem1.74.0 \
    libboost-thread1.74.0 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Create application user
RUN groupadd -r cba && useradd -r -g cba cba

# Copy application from builder
COPY --from=builder /app /

# Create directories
RUN mkdir -p /etc/cba /var/log/cba /var/lib/cba && \
    chown -R cba:cba /etc/cba /var/log/cba /var/lib/cba

# Copy configuration
COPY config/production.json /etc/cba/config.json
COPY scripts/docker-entrypoint.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/docker-entrypoint.sh

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=60s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# Switch to non-root user
USER cba

# Expose ports
EXPOSE 8080 8443

# Set entrypoint
ENTRYPOINT ["/usr/local/bin/docker-entrypoint.sh"]
CMD ["/usr/local/bin/cba-server"]
```

### Docker Entrypoint Script

```bash
#!/bin/bash
# scripts/docker-entrypoint.sh

set -e

# Default configuration
CONFIG_FILE=${CONFIG_FILE:-"/etc/cba/config.json"}
LOG_LEVEL=${LOG_LEVEL:-"INFO"}
PORT=${PORT:-8080}
SSL_PORT=${SSL_PORT:-8443}

# Environment variable substitution in config
if [ -f "$CONFIG_FILE" ]; then
    # Replace environment variables in config file
    envsubst < "$CONFIG_FILE" > "/tmp/config.json"
    mv "/tmp/config.json" "$CONFIG_FILE"
fi

# Wait for dependencies
if [ -n "$WAIT_FOR_HOSTS" ]; then
    echo "Waiting for dependencies: $WAIT_FOR_HOSTS"
    for host in $(echo $WAIT_FOR_HOSTS | tr "," " "); do
        echo "Waiting for $host..."
        while ! nc -z ${host/:/ } 2>/dev/null; do
            sleep 1
        done
        echo "$host is ready"
    done
fi

# Initialize application
echo "Starting CoreBaseApplication..."
echo "Config file: $CONFIG_FILE"
echo "Log level: $LOG_LEVEL"
echo "Port: $PORT"
echo "SSL Port: $SSL_PORT"

# Execute the main command
exec "$@" \
    --config="$CONFIG_FILE" \
    --log-level="$LOG_LEVEL" \
    --port="$PORT" \
    --ssl-port="$SSL_PORT"
```

### Build and Push Script

```bash
#!/bin/bash
# scripts/build-docker.sh

set -e

# Configuration
IMAGE_NAME="cba/corebaseapplication"
VERSION=$(git describe --tags --always)
REGISTRY="your-registry.com"

# Build image
echo "Building Docker image: $IMAGE_NAME:$VERSION"
docker build \
    --tag "$IMAGE_NAME:$VERSION" \
    --tag "$IMAGE_NAME:latest" \
    --build-arg VERSION="$VERSION" \
    --build-arg BUILD_DATE="$(date -u +'%Y-%m-%dT%H:%M:%SZ')" \
    --build-arg VCS_REF="$(git rev-parse HEAD)" \
    .

# Security scan
echo "Scanning image for vulnerabilities..."
docker run --rm -v /var/run/docker.sock:/var/run/docker.sock \
    aquasec/trivy image "$IMAGE_NAME:$VERSION"

# Push to registry
if [ "$1" = "--push" ]; then
    echo "Pushing to registry: $REGISTRY"
    docker tag "$IMAGE_NAME:$VERSION" "$REGISTRY/$IMAGE_NAME:$VERSION"
    docker tag "$IMAGE_NAME:latest" "$REGISTRY/$IMAGE_NAME:latest"
    
    docker push "$REGISTRY/$IMAGE_NAME:$VERSION"
    docker push "$REGISTRY/$IMAGE_NAME:latest"
    
    echo "Image pushed successfully"
fi

echo "Build completed: $IMAGE_NAME:$VERSION"
```

## Kubernetes Deployment

### Namespace and RBAC

```yaml
# k8s/namespace.yaml
apiVersion: v1
kind: Namespace
metadata:
  name: cba-production
  labels:
    name: cba-production
    environment: production

---
apiVersion: v1
kind: ServiceAccount
metadata:
  name: cba-service-account
  namespace: cba-production

---
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  namespace: cba-production
  name: cba-role
rules:
- apiGroups: [""]
  resources: ["configmaps", "secrets"]
  verbs: ["get", "list", "watch"]
- apiGroups: [""]
  resources: ["pods"]
  verbs: ["get", "list"]

---
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: cba-role-binding
  namespace: cba-production
subjects:
- kind: ServiceAccount
  name: cba-service-account
  namespace: cba-production
roleRef:
  kind: Role
  name: cba-role
  apiGroup: rbac.authorization.k8s.io
```

### ConfigMap and Secrets

```yaml
# k8s/configmap.yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: cba-config
  namespace: cba-production
data:
  config.json: |
    {
      "server": {
        "host": "0.0.0.0",
        "port": 8080,
        "ssl_port": 8443,
        "worker_threads": 4
      },
      "database": {
        "host": "${DATABASE_HOST}",
        "port": ${DATABASE_PORT},
        "name": "${DATABASE_NAME}",
        "pool_size": 20
      },
      "logging": {
        "level": "INFO",
        "format": "json",
        "appenders": [
          {
            "type": "console",
            "colored": false
          },
          {
            "type": "file",
            "path": "/var/log/cba/app.log",
            "max_size": "100MB",
            "max_files": 10
          }
        ]
      },
      "monitoring": {
        "enabled": true,
        "metrics_port": 9090,
        "health_check_interval": 30
      }
    }

---
apiVersion: v1
kind: Secret
metadata:
  name: cba-secrets
  namespace: cba-production
type: Opaque
data:
  database-username: Y2JhX3VzZXI=  # base64 encoded
  database-password: c2VjdXJlX3Bhc3N3b3Jk  # base64 encoded
  api-key: eW91cl9hcGlfa2V5X2hlcmU=  # base64 encoded
  ssl-cert: LS0tLS1CRUdJTi4uLi4=  # base64 encoded certificate
  ssl-key: LS0tLS1CRUdJTi4uLi4=   # base64 encoded private key
```

### Deployment

```yaml
# k8s/deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: cba-deployment
  namespace: cba-production
  labels:
    app: cba
    version: v1.0.0
spec:
  replicas: 3
  strategy:
    type: RollingUpdate
    rollingUpdate:
      maxSurge: 1
      maxUnavailable: 0
  selector:
    matchLabels:
      app: cba
  template:
    metadata:
      labels:
        app: cba
        version: v1.0.0
      annotations:
        prometheus.io/scrape: "true"
        prometheus.io/port: "9090"
        prometheus.io/path: "/metrics"
    spec:
      serviceAccountName: cba-service-account
      securityContext:
        runAsNonRoot: true
        runAsUser: 1000
        runAsGroup: 1000
        fsGroup: 1000
      containers:
      - name: cba
        image: your-registry.com/cba/corebaseapplication:v1.0.0
        imagePullPolicy: Always
        ports:
        - containerPort: 8080
          name: http
          protocol: TCP
        - containerPort: 8443
          name: https
          protocol: TCP
        - containerPort: 9090
          name: metrics
          protocol: TCP
        env:
        - name: DATABASE_HOST
          value: "postgres-service.database.svc.cluster.local"
        - name: DATABASE_PORT
          value: "5432"
        - name: DATABASE_NAME
          value: "cba_production"
        - name: DATABASE_USERNAME
          valueFrom:
            secretKeyRef:
              name: cba-secrets
              key: database-username
        - name: DATABASE_PASSWORD
          valueFrom:
            secretKeyRef:
              name: cba-secrets
              key: database-password
        - name: API_KEY
          valueFrom:
            secretKeyRef:
              name: cba-secrets
              key: api-key
        volumeMounts:
        - name: config-volume
          mountPath: /etc/cba
          readOnly: true
        - name: ssl-certs
          mountPath: /etc/ssl/certs/cba
          readOnly: true
        - name: logs
          mountPath: /var/log/cba
        - name: data
          mountPath: /var/lib/cba
        resources:
          requests:
            memory: "512Mi"
            cpu: "250m"
          limits:
            memory: "1Gi"
            cpu: "500m"
        livenessProbe:
          httpGet:
            path: /health
            port: 8080
          initialDelaySeconds: 60
          periodSeconds: 30
          timeoutSeconds: 10
          failureThreshold: 3
        readinessProbe:
          httpGet:
            path: /ready
            port: 8080
          initialDelaySeconds: 30
          periodSeconds: 10
          timeoutSeconds: 5
          failureThreshold: 3
        startupProbe:
          httpGet:
            path: /health
            port: 8080
          initialDelaySeconds: 30
          periodSeconds: 10
          timeoutSeconds: 5
          failureThreshold: 30
        securityContext:
          allowPrivilegeEscalation: false
          readOnlyRootFilesystem: true
          capabilities:
            drop:
            - ALL
      volumes:
      - name: config-volume
        configMap:
          name: cba-config
      - name: ssl-certs
        secret:
          secretName: cba-secrets
          items:
          - key: ssl-cert
            path: tls.crt
          - key: ssl-key
            path: tls.key
      - name: logs
        emptyDir: {}
      - name: data
        persistentVolumeClaim:
          claimName: cba-data-pvc
      nodeSelector:
        kubernetes.io/os: linux
      tolerations:
      - key: "node-role.kubernetes.io/worker"
        operator: "Equal"
        value: "true"
        effect: "NoSchedule"
      affinity:
        podAntiAffinity:
          preferredDuringSchedulingIgnoredDuringExecution:
          - weight: 100
            podAffinityTerm:
              labelSelector:
                matchExpressions:
                - key: app
                  operator: In
                  values:
                  - cba
              topologyKey: kubernetes.io/hostname
```

### Services and Ingress

```yaml
# k8s/service.yaml
apiVersion: v1
kind: Service
metadata:
  name: cba-service
  namespace: cba-production
  labels:
    app: cba
spec:
  type: ClusterIP
  ports:
  - port: 80
    targetPort: 8080
    protocol: TCP
    name: http
  - port: 443
    targetPort: 8443
    protocol: TCP
    name: https
  - port: 9090
    targetPort: 9090
    protocol: TCP
    name: metrics
  selector:
    app: cba

---
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: cba-ingress
  namespace: cba-production
  annotations:
    kubernetes.io/ingress.class: nginx
    nginx.ingress.kubernetes.io/ssl-redirect: "true"
    nginx.ingress.kubernetes.io/force-ssl-redirect: "true"
    nginx.ingress.kubernetes.io/backend-protocol: "HTTP"
    nginx.ingress.kubernetes.io/proxy-body-size: "10m"
    nginx.ingress.kubernetes.io/rate-limit: "100"
    nginx.ingress.kubernetes.io/rate-limit-window: "1m"
    cert-manager.io/cluster-issuer: "letsencrypt-prod"
spec:
  tls:
  - hosts:
    - api.yourdomain.com
    secretName: cba-tls-secret
  rules:
  - host: api.yourdomain.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: cba-service
            port:
              number: 80
```

### Persistent Volume

```yaml
# k8s/pvc.yaml
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: cba-data-pvc
  namespace: cba-production
spec:
  accessModes:
    - ReadWriteOnce
  storageClassName: fast-ssd
  resources:
    requests:
      storage: 50Gi
```

### Horizontal Pod Autoscaler

```yaml
# k8s/hpa.yaml
apiVersion: autoscaling/v2
kind: HorizontalPodAutoscaler
metadata:
  name: cba-hpa
  namespace: cba-production
spec:
  scaleTargetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: cba-deployment
  minReplicas: 3
  maxReplicas: 10
  metrics:
  - type: Resource
    resource:
      name: cpu
      target:
        type: Utilization
        averageUtilization: 70
  - type: Resource
    resource:
      name: memory
      target:
        type: Utilization
        averageUtilization: 80
  behavior:
    scaleDown:
      stabilizationWindowSeconds: 300
      policies:
      - type: Percent
        value: 10
        periodSeconds: 60
    scaleUp:
      stabilizationWindowSeconds: 60
      policies:
      - type: Percent
        value: 50
        periodSeconds: 60
      - type: Pods
        value: 2
        periodSeconds: 60
      selectPolicy: Max
```

## Docker Compose

### Production Docker Compose

```yaml
# docker-compose.prod.yml
version: '3.8'

services:
  cba-app:
    image: your-registry.com/cba/corebaseapplication:latest
    container_name: cba-app
    restart: unless-stopped
    ports:
      - "8080:8080"
      - "8443:8443"
      - "9090:9090"
    environment:
      - DATABASE_HOST=postgres
      - DATABASE_PORT=5432
      - DATABASE_NAME=cba_production
      - DATABASE_USERNAME_FILE=/run/secrets/db_username
      - DATABASE_PASSWORD_FILE=/run/secrets/db_password
      - LOG_LEVEL=INFO
      - WAIT_FOR_HOSTS=postgres:5432,redis:6379
    volumes:
      - ./config/production.json:/etc/cba/config.json:ro
      - ./ssl:/etc/ssl/certs/cba:ro
      - cba-logs:/var/log/cba
      - cba-data:/var/lib/cba
    secrets:
      - db_username
      - db_password
      - api_key
    networks:
      - cba-network
    depends_on:
      postgres:
        condition: service_healthy
      redis:
        condition: service_healthy
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8080/health"]
      interval: 30s
      timeout: 10s
      retries: 3
      start_period: 60s
    deploy:
      resources:
        limits:
          cpus: '1.0'
          memory: 1G
        reservations:
          cpus: '0.5'
          memory: 512M
      restart_policy:
        condition: on-failure
        delay: 5s
        max_attempts: 3
        window: 120s

  postgres:
    image: postgres:15-alpine
    container_name: cba-postgres
    restart: unless-stopped
    environment:
      - POSTGRES_DB=cba_production
      - POSTGRES_USER_FILE=/run/secrets/db_username
      - POSTGRES_PASSWORD_FILE=/run/secrets/db_password
      - POSTGRES_INITDB_ARGS=--auth-host=scram-sha-256
    volumes:
      - postgres-data:/var/lib/postgresql/data
      - ./init-scripts:/docker-entrypoint-initdb.d:ro
    secrets:
      - db_username
      - db_password
    networks:
      - cba-network
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U $$(cat /run/secrets/db_username) -d cba_production"]
      interval: 10s
      timeout: 5s
      retries: 5
    deploy:
      resources:
        limits:
          cpus: '0.5'
          memory: 512M

  redis:
    image: redis:7-alpine
    container_name: cba-redis
    restart: unless-stopped
    command: redis-server --requirepass $$(cat /run/secrets/redis_password)
    volumes:
      - redis-data:/data
    secrets:
      - redis_password
    networks:
      - cba-network
    healthcheck:
      test: ["CMD", "redis-cli", "--no-auth-warning", "-a", "$$(cat /run/secrets/redis_password)", "ping"]
      interval: 10s
      timeout: 5s
      retries: 5
    deploy:
      resources:
        limits:
          cpus: '0.25'
          memory: 256M

  nginx:
    image: nginx:alpine
    container_name: cba-nginx
    restart: unless-stopped
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./nginx/nginx.conf:/etc/nginx/nginx.conf:ro
      - ./nginx/conf.d:/etc/nginx/conf.d:ro
      - ./ssl:/etc/nginx/ssl:ro
      - nginx-logs:/var/log/nginx
    networks:
      - cba-network
    depends_on:
      - cba-app
    healthcheck:
      test: ["CMD", "wget", "--no-verbose", "--tries=1", "--spider", "http://localhost/health"]
      interval: 30s
      timeout: 10s
      retries: 3

  prometheus:
    image: prom/prometheus:latest
    container_name: cba-prometheus
    restart: unless-stopped
    ports:
      - "9091:9090"
    volumes:
      - ./monitoring/prometheus.yml:/etc/prometheus/prometheus.yml:ro
      - prometheus-data:/prometheus
    command:
      - '--config.file=/etc/prometheus/prometheus.yml'
      - '--storage.tsdb.path=/prometheus'
      - '--web.console.libraries=/etc/prometheus/console_libraries'
      - '--web.console.templates=/etc/prometheus/consoles'
      - '--storage.tsdb.retention.time=30d'
      - '--web.enable-lifecycle'
    networks:
      - cba-network

  grafana:
    image: grafana/grafana:latest
    container_name: cba-grafana
    restart: unless-stopped
    ports:
      - "3000:3000"
    environment:
      - GF_SECURITY_ADMIN_PASSWORD_FILE=/run/secrets/grafana_password
      - GF_INSTALL_PLUGINS=grafana-piechart-panel
    volumes:
      - grafana-data:/var/lib/grafana
      - ./monitoring/grafana/dashboards:/etc/grafana/provisioning/dashboards:ro
      - ./monitoring/grafana/datasources:/etc/grafana/provisioning/datasources:ro
    secrets:
      - grafana_password
    networks:
      - cba-network
    depends_on:
      - prometheus

secrets:
  db_username:
    file: ./secrets/db_username.txt
  db_password:
    file: ./secrets/db_password.txt
  redis_password:
    file: ./secrets/redis_password.txt
  api_key:
    file: ./secrets/api_key.txt
  grafana_password:
    file: ./secrets/grafana_password.txt

volumes:
  cba-logs:
    driver: local
  cba-data:
    driver: local
  postgres-data:
    driver: local
  redis-data:
    driver: local
  nginx-logs:
    driver: local
  prometheus-data:
    driver: local
  grafana-data:
    driver: local

networks:
  cba-network:
    driver: bridge
    ipam:
      config:
        - subnet: 172.20.0.0/16
```

### Nginx Configuration

```nginx
# nginx/conf.d/cba.conf
upstream cba_backend {
    least_conn;
    server cba-app:8080 max_fails=3 fail_timeout=30s;
    keepalive 32;
}

# Rate limiting
limit_req_zone $binary_remote_addr zone=api:10m rate=10r/s;
limit_req_zone $binary_remote_addr zone=login:10m rate=1r/s;

# SSL configuration
ssl_protocols TLSv1.2 TLSv1.3;
ssl_ciphers ECDHE-RSA-AES256-GCM-SHA512:DHE-RSA-AES256-GCM-SHA512:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES256-GCM-SHA384;
ssl_prefer_server_ciphers off;
ssl_session_cache shared:SSL:10m;
ssl_session_timeout 10m;

server {
    listen 80;
    server_name api.yourdomain.com;
    
    # Redirect HTTP to HTTPS
    return 301 https://$server_name$request_uri;
}

server {
    listen 443 ssl http2;
    server_name api.yourdomain.com;
    
    # SSL certificates
    ssl_certificate /etc/nginx/ssl/tls.crt;
    ssl_certificate_key /etc/nginx/ssl/tls.key;
    
    # Security headers
    add_header Strict-Transport-Security "max-age=31536000; includeSubDomains" always;
    add_header X-Frame-Options DENY always;
    add_header X-Content-Type-Options nosniff always;
    add_header X-XSS-Protection "1; mode=block" always;
    add_header Referrer-Policy "strict-origin-when-cross-origin" always;
    
    # Logging
    access_log /var/log/nginx/cba_access.log;
    error_log /var/log/nginx/cba_error.log;
    
    # Health check endpoint (no rate limiting)
    location /health {
        proxy_pass http://cba_backend;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        
        access_log off;
    }
    
    # API endpoints with rate limiting
    location /api/ {
        limit_req zone=api burst=20 nodelay;
        
        proxy_pass http://cba_backend;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        
        # Timeouts
        proxy_connect_timeout 5s;
        proxy_send_timeout 60s;
        proxy_read_timeout 60s;
        
        # Buffering
        proxy_buffering on;
        proxy_buffer_size 4k;
        proxy_buffers 8 4k;
    }
    
    # Login endpoint with stricter rate limiting
    location /api/auth/login {
        limit_req zone=login burst=5 nodelay;
        
        proxy_pass http://cba_backend;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
    
    # Static files (if any)
    location /static/ {
        expires 1y;
        add_header Cache-Control "public, immutable";
        
        proxy_pass http://cba_backend;
        proxy_set_header Host $host;
    }
    
    # Metrics endpoint (internal only)
    location /metrics {
        allow 172.20.0.0/16;  # Docker network
        deny all;
        
        proxy_pass http://cba_backend:9090;
        proxy_set_header Host $host;
    }
}
```

## Load Balancing

### HAProxy Configuration

```haproxy
# haproxy/haproxy.cfg
global
    daemon
    user haproxy
    group haproxy
    
    # SSL configuration
    ssl-default-bind-ciphers ECDHE-RSA-AES256-GCM-SHA512:DHE-RSA-AES256-GCM-SHA512:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES256-GCM-SHA384
    ssl-default-bind-options ssl-min-ver TLSv1.2 no-tls-tickets
    
    # Logging
    log stdout local0
    
    # Stats
    stats socket /var/run/haproxy/admin.sock mode 660 level admin
    stats timeout 30s

defaults
    mode http
    timeout connect 5000ms
    timeout client 50000ms
    timeout server 50000ms
    
    # Logging
    option httplog
    log global
    
    # Health checks
    option httpchk GET /health
    
    # Error pages
    errorfile 400 /etc/haproxy/errors/400.http
    errorfile 403 /etc/haproxy/errors/403.http
    errorfile 408 /etc/haproxy/errors/408.http
    errorfile 500 /etc/haproxy/errors/500.http
    errorfile 502 /etc/haproxy/errors/502.http
    errorfile 503 /etc/haproxy/errors/503.http
    errorfile 504 /etc/haproxy/errors/504.http

# Frontend for HTTP (redirect to HTTPS)
frontend http_frontend
    bind *:80
    redirect scheme https code 301 if !{ ssl_fc }

# Frontend for HTTPS
frontend https_frontend
    bind *:443 ssl crt /etc/ssl/certs/cba/
    
    # Security headers
    http-response set-header Strict-Transport-Security "max-age=31536000; includeSubDomains"
    http-response set-header X-Frame-Options DENY
    http-response set-header X-Content-Type-Options nosniff
    http-response set-header X-XSS-Protection "1; mode=block"
    
    # Rate limiting
    stick-table type ip size 100k expire 30s store http_req_rate(10s)
    http-request track-sc0 src
    http-request deny if { sc_http_req_rate(0) gt 20 }
    
    # Route to backend
    default_backend cba_backend

# Backend servers
backend cba_backend
    balance roundrobin
    
    # Health check
    option httpchk GET /health
    http-check expect status 200
    
    # Servers
    server cba1 cba-app-1:8080 check inter 10s fall 3 rise 2
    server cba2 cba-app-2:8080 check inter 10s fall 3 rise 2
    server cba3 cba-app-3:8080 check inter 10s fall 3 rise 2

# Stats interface
listen stats
    bind *:8404
    stats enable
    stats uri /stats
    stats refresh 30s
    stats admin if TRUE
```

## Monitoring and Observability

### Prometheus Configuration

```yaml
# monitoring/prometheus.yml
global:
  scrape_interval: 15s
  evaluation_interval: 15s

rule_files:
  - "alert_rules.yml"

alerting:
  alertmanagers:
    - static_configs:
        - targets:
          - alertmanager:9093

scrape_configs:
  - job_name: 'cba-application'
    static_configs:
      - targets: ['cba-app:9090']
    scrape_interval: 10s
    metrics_path: /metrics
    
  - job_name: 'postgres'
    static_configs:
      - targets: ['postgres-exporter:9187']
    
  - job_name: 'redis'
    static_configs:
      - targets: ['redis-exporter:9121']
    
  - job_name: 'nginx'
    static_configs:
      - targets: ['nginx-exporter:9113']
    
  - job_name: 'node-exporter'
    static_configs:
      - targets: ['node-exporter:9100']
```

### Alert Rules

```yaml
# monitoring/alert_rules.yml
groups:
  - name: cba_alerts
    rules:
      - alert: HighCPUUsage
        expr: cpu_usage_percent > 80
        for: 5m
        labels:
          severity: warning
        annotations:
          summary: "High CPU usage detected"
          description: "CPU usage is above 80% for more than 5 minutes"
      
      - alert: HighMemoryUsage
        expr: memory_usage_mb > 800
        for: 5m
        labels:
          severity: warning
        annotations:
          summary: "High memory usage detected"
          description: "Memory usage is above 800MB for more than 5 minutes"
      
      - alert: HighErrorRate
        expr: rate(total_errors_total[5m]) / rate(total_requests_total[5m]) > 0.05
        for: 2m
        labels:
          severity: critical
        annotations:
          summary: "High error rate detected"
          description: "Error rate is above 5% for more than 2 minutes"
      
      - alert: ServiceDown
        expr: up == 0
        for: 1m
        labels:
          severity: critical
        annotations:
          summary: "Service is down"
          description: "{% raw %}{{ $labels.instance }}{% endraw %} has been down for more than 1 minute"
      
      - alert: DatabaseConnectionFailure
        expr: database_connections_failed_total > 0
        for: 1m
        labels:
          severity: critical
        annotations:
          summary: "Database connection failures"
          description: "Database connection failures detected"
```

### Grafana Dashboard

```json
{
  "dashboard": {
    "id": null,
    "title": "CBA Application Dashboard",
    "tags": ["cba", "production"],
    "timezone": "browser",
    "panels": [
      {
        "id": 1,
        "title": "Request Rate",
        "type": "graph",
        "targets": [
          {
            "expr": "rate(total_requests_total[5m])",
            "legendFormat": "Requests/sec"
          }
        ],
        "yAxes": [
          {
            "label": "Requests/sec",
            "min": 0
          }
        ]
      },
      {
        "id": 2,
        "title": "Error Rate",
        "type": "graph",
        "targets": [
          {
            "expr": "rate(total_errors_total[5m]) / rate(total_requests_total[5m]) * 100",
            "legendFormat": "Error Rate %"
          }
        ],
        "yAxes": [
          {
            "label": "Percentage",
            "min": 0,
            "max": 100
          }
        ]
      },
      {
        "id": 3,
        "title": "Response Time",
        "type": "graph",
        "targets": [
          {
            "expr": "histogram_quantile(0.95, rate(http_request_duration_seconds_bucket[5m]))",
            "legendFormat": "95th percentile"
          },
          {
            "expr": "histogram_quantile(0.50, rate(http_request_duration_seconds_bucket[5m]))",
            "legendFormat": "50th percentile"
          }
        ]
      },
      {
        "id": 4,
        "title": "System Resources",
        "type": "graph",
        "targets": [
          {
            "expr": "cpu_usage_percent",
            "legendFormat": "CPU %"
          },
          {
            "expr": "memory_usage_mb / 1024",
            "legendFormat": "Memory GB"
          }
        ]
      }
    ],
    "time": {
      "from": "now-1h",
      "to": "now"
    },
    "refresh": "5s"
  }
}
```

## Security Hardening

### Security Checklist

```bash
#!/bin/bash
# scripts/security-hardening.sh

set -e

echo "Starting security hardening..."

# 1. Update system packages
sudo apt-get update && sudo apt-get upgrade -y

# 2. Install security tools
sudo apt-get install -y fail2ban ufw rkhunter chkrootkit

# 3. Configure firewall
sudo ufw default deny incoming
sudo ufw default allow outgoing
sudo ufw allow ssh
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp
sudo ufw --force enable

# 4. Configure fail2ban
sudo cp /etc/fail2ban/jail.conf /etc/fail2ban/jail.local
sudo systemctl enable fail2ban
sudo systemctl start fail2ban

# 5. Secure SSH
sudo sed -i 's/#PermitRootLogin yes/PermitRootLogin no/' /etc/ssh/sshd_config
sudo sed -i 's/#PasswordAuthentication yes/PasswordAuthentication no/' /etc/ssh/sshd_config
sudo systemctl restart ssh

# 6. Set up log monitoring
sudo apt-get install -y logwatch
echo "logwatch --output mail --mailto admin@yourdomain.com --detail high" | sudo tee /etc/cron.daily/00logwatch

# 7. Configure automatic security updates
sudo apt-get install -y unattended-upgrades
echo 'Unattended-Upgrade::Automatic-Reboot "false";' | sudo tee -a /etc/apt/apt.conf.d/50unattended-upgrades

# 8. Harden kernel parameters
cat << EOF | sudo tee -a /etc/sysctl.conf
# Network security
net.ipv4.conf.default.rp_filter=1
net.ipv4.conf.all.rp_filter=1
net.ipv4.tcp_syncookies=1
net.ipv4.conf.all.accept_redirects=0
net.ipv6.conf.all.accept_redirects=0
net.ipv4.conf.all.send_redirects=0
net.ipv4.conf.all.accept_source_route=0
net.ipv6.conf.all.accept_source_route=0
net.ipv4.conf.all.log_martians=1

# Memory protection
kernel.exec-shield=1
kernel.randomize_va_space=2
EOF

sudo sysctl -p

echo "Security hardening completed"
```

### SSL/TLS Configuration

```bash
#!/bin/bash
# scripts/setup-ssl.sh

set -e

DOMAIN="api.yourdomain.com"
SSL_DIR="/etc/ssl/certs/cba"

# Create SSL directory
sudo mkdir -p $SSL_DIR

# Generate strong DH parameters
sudo openssl dhparam -out $SSL_DIR/dhparam.pem 2048

# Generate self-signed certificate for development
if [ "$1" = "--self-signed" ]; then
    sudo openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
        -keyout $SSL_DIR/tls.key \
        -out $SSL_DIR/tls.crt \
        -subj "/C=US/ST=State/L=City/O=Organization/CN=$DOMAIN"
else
    # Use Let's Encrypt for production
    sudo apt-get install -y certbot
    sudo certbot certonly --standalone -d $DOMAIN
    
    # Copy certificates
    sudo cp /etc/letsencrypt/live/$DOMAIN/fullchain.pem $SSL_DIR/tls.crt
    sudo cp /etc/letsencrypt/live/$DOMAIN/privkey.pem $SSL_DIR/tls.key
    
    # Set up auto-renewal
    echo "0 12 * * * /usr/bin/certbot renew --quiet" | sudo crontab -
fi

# Set proper permissions
sudo chmod 600 $SSL_DIR/tls.key
sudo chmod 644 $SSL_DIR/tls.crt
sudo chown root:root $SSL_DIR/*

echo "SSL certificates configured successfully"
```

## Performance Tuning

### System Optimization

```bash
#!/bin/bash
# scripts/performance-tuning.sh

set -e

echo "Starting performance tuning..."

# 1. Kernel parameters for high performance
cat << EOF | sudo tee -a /etc/sysctl.conf
# Network performance
net.core.rmem_max = 134217728
net.core.wmem_max = 134217728
net.ipv4.tcp_rmem = 4096 65536 134217728
net.ipv4.tcp_wmem = 4096 65536 134217728
net.core.netdev_max_backlog = 5000
net.ipv4.tcp_congestion_control = bbr

# File system
fs.file-max = 2097152
fs.nr_open = 1048576

# Virtual memory
vm.swappiness = 10
vm.dirty_ratio = 15
vm.dirty_background_ratio = 5
EOF

# 2. Increase file descriptor limits
cat << EOF | sudo tee -a /etc/security/limits.conf
* soft nofile 65536
* hard nofile 65536
* soft nproc 65536
* hard nproc 65536
EOF

# 3. Configure systemd limits
sudo mkdir -p /etc/systemd/system.conf.d
cat << EOF | sudo tee /etc/systemd/system.conf.d/limits.conf
[Manager]
DefaultLimitNOFILE=65536
DefaultLimitNPROC=65536
EOF

# 4. Optimize disk I/O
echo 'ACTION=="add|change", KERNEL=="sd[a-z]", ATTR{queue/rotational}=="0", ATTR{queue/scheduler}="mq-deadline"' | sudo tee /etc/udev/rules.d/60-ioschedulers.rules
echo 'ACTION=="add|change", KERNEL=="sd[a-z]", ATTR{queue/rotational}=="1", ATTR{queue/scheduler}="bfq"' | sudo tee -a /etc/udev/rules.d/60-ioschedulers.rules

# 5. Apply changes
sudo sysctl -p
sudo systemctl daemon-reload

echo "Performance tuning completed. Reboot required for all changes to take effect."
```

### Application Configuration

```json
{
  "server": {
    "worker_threads": 8,
    "max_connections": 1000,
    "keep_alive_timeout": 60,
    "request_timeout": 30,
    "max_request_size": "10MB"
  },
  "database": {
    "pool_size": 20,
    "max_idle_connections": 5,
    "connection_timeout": 30,
    "query_timeout": 60,
    "prepared_statement_cache_size": 100
  },
  "cache": {
    "enabled": true,
    "type": "redis",
    "ttl": 3600,
    "max_memory": "512MB",
    "eviction_policy": "allkeys-lru"
  },
  "performance": {
    "enable_compression": true,
    "compression_level": 6,
    "enable_http2": true,
    "enable_keep_alive": true,
    "buffer_size": "64KB"
  }
}
```

## Backup and Recovery

### Backup Strategy

```bash
#!/bin/bash
# scripts/backup.sh

set -e

# Configuration
BACKUP_DIR="/backup/cba"
RETENTION_DAYS=30
S3_BUCKET="your-backup-bucket"
DATE=$(date +%Y%m%d_%H%M%S)

# Create backup directory
mkdir -p $BACKUP_DIR

echo "Starting backup process..."

# 1. Database backup
echo "Backing up database..."
pg_dump -h postgres -U cba_user -d cba_production | gzip > $BACKUP_DIR/database_$DATE.sql.gz

# 2. Application data backup
echo "Backing up application data..."
tar -czf $BACKUP_DIR/app_data_$DATE.tar.gz /var/lib/cba

# 3. Configuration backup
echo "Backing up configuration..."
tar -czf $BACKUP_DIR/config_$DATE.tar.gz /etc/cba

# 4. SSL certificates backup
echo "Backing up SSL certificates..."
tar -czf $BACKUP_DIR/ssl_$DATE.tar.gz /etc/ssl/certs/cba

# 5. Upload to S3 (if configured)
if [ -n "$S3_BUCKET" ]; then
    echo "Uploading backups to S3..."
    aws s3 sync $BACKUP_DIR s3://$S3_BUCKET/cba-backups/
fi

# 6. Cleanup old backups
echo "Cleaning up old backups..."
find $BACKUP_DIR -name "*" -type f -mtime +$RETENTION_DAYS -delete

echo "Backup completed successfully"
```

### Recovery Procedures

```bash
#!/bin/bash
# scripts/restore.sh

set -e

BACKUP_DATE=$1
BACKUP_DIR="/backup/cba"

if [ -z "$BACKUP_DATE" ]; then
    echo "Usage: $0 <backup_date>"
    echo "Available backups:"
    ls -la $BACKUP_DIR/database_*.sql.gz | awk '{print $9}' | sed 's/.*database_\(.*\)\.sql\.gz/\1/'
    exit 1
fi

echo "Starting restore process for backup: $BACKUP_DATE"

# 1. Stop application
echo "Stopping application..."
docker-compose down

# 2. Restore database
echo "Restoring database..."
gunzip -c $BACKUP_DIR/database_$BACKUP_DATE.sql.gz | psql -h postgres -U cba_user -d cba_production

# 3. Restore application data
echo "Restoring application data..."
tar -xzf $BACKUP_DIR/app_data_$BACKUP_DATE.tar.gz -C /

# 4. Restore configuration
echo "Restoring configuration..."
tar -xzf $BACKUP_DIR/config_$BACKUP_DATE.tar.gz -C /

# 5. Restore SSL certificates
echo "Restoring SSL certificates..."
tar -xzf $BACKUP_DIR/ssl_$BACKUP_DATE.tar.gz -C /

# 6. Start application
echo "Starting application..."
docker-compose up -d

# 7. Verify restore
echo "Verifying restore..."
sleep 30
curl -f http://localhost:8080/health || echo "Health check failed"

echo "Restore completed successfully"
```

## CI/CD Pipeline

### GitHub Actions Workflow

```yaml
# .github/workflows/deploy-production.yml
name: Deploy to Production

on:
  push:
    tags:
      - 'v*'

env:
  REGISTRY: ghcr.io
  IMAGE_NAME: {% raw %}${{ github.repository }}{% endraw %}

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Setup C++
        uses: actions/setup-cpp@v1
        with:
          compiler: gcc-11
          
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential cmake libssl-dev libcurl4-openssl-dev libjsoncpp-dev libboost-all-dev
          
      - name: Build and test
        run: |
          mkdir build
          cd build
          cmake .. -DCMAKE_BUILD_TYPE=Release -DCBA_ENABLE_TESTS=ON
          make -j$(nproc)
          ctest --output-on-failure
          
      - name: Security scan
        run: |
          # Run static analysis
          cppcheck --enable=all --error-exitcode=1 src/
          
  build-and-push:
    needs: test
    runs-on: ubuntu-latest
    permissions:
      contents: read
      packages: write
      
    steps:
      - uses: actions/checkout@v4
      
      - name: Log in to Container Registry
        uses: docker/login-action@v3
        with:
          registry: {% raw %}${{ env.REGISTRY }}{% endraw %}
        username: {% raw %}${{ github.actor }}{% endraw %}
        password: {% raw %}${{ secrets.GITHUB_TOKEN }}{% endraw %}
          
      - name: Extract metadata
        id: meta
        uses: docker/metadata-action@v5
        with:
          images: {% raw %}${{ env.REGISTRY }}/${{ env.IMAGE_NAME }}{% endraw %}
          tags: |
            type=ref,event=tag
            type=raw,value=latest,enable={% raw %}{{is_default_branch}}{% endraw %}
            
      - name: Build and push Docker image
        uses: docker/build-push-action@v5
        with:
          context: .
          push: true
          tags: {% raw %}${{ steps.meta.outputs.tags }}{% endraw %}
        labels: {% raw %}${{ steps.meta.outputs.labels }}{% endraw %}
          
  deploy:
    needs: build-and-push
    runs-on: ubuntu-latest
    environment: production
    
    steps:
      - uses: actions/checkout@v4
      
      - name: Setup kubectl
        uses: azure/setup-kubectl@v3
        with:
          version: 'v1.28.0'
          
      - name: Configure kubectl
        run: |
          echo "{% raw %}${{ secrets.KUBECONFIG }}{% endraw %}" | base64 -d > kubeconfig
          export KUBECONFIG=kubeconfig
          
      - name: Deploy to Kubernetes
        run: |
          export KUBECONFIG=kubeconfig
          
          # Update image tag
          kubectl set image deployment/cba-deployment cba={% raw %}${{ env.REGISTRY }}/${{ env.IMAGE_NAME }}:${{ github.ref_name }}{% endraw %} -n cba-production
          
          # Wait for rollout
          kubectl rollout status deployment/cba-deployment -n cba-production --timeout=600s
          
          # Verify deployment
          kubectl get pods -n cba-production
          
      - name: Run smoke tests
        run: |
          # Wait for service to be ready
          sleep 60
          
          # Get service URL
          SERVICE_URL=$(kubectl get ingress cba-ingress -n cba-production -o jsonpath='{.spec.rules[0].host}')
          
          # Run smoke tests
          curl -f https://$SERVICE_URL/health || exit 1
          curl -f https://$SERVICE_URL/api/version || exit 1
          
          echo "Deployment successful!"
          
      - name: Notify deployment
        if: always()
        uses: 8398a7/action-slack@v3
        with:
          status: {% raw %}${{ job.status }}{% endraw %}
          channel: '#deployments'
          webhook_url: {% raw %}${{ secrets.SLACK_WEBHOOK }}{% endraw %}
```

### Deployment Scripts

```bash
#!/bin/bash
# scripts/deploy-production.sh

set -e

# Configuration
ENVIRONMENT="production"
NAMESPACE="cba-production"
IMAGE_TAG=${1:-"latest"}
REGISTRY="ghcr.io/seregonwar"
IMAGE_NAME="corebaseapplication"

echo "Deploying CBA to production environment..."
echo "Image: $REGISTRY/$IMAGE_NAME:$IMAGE_TAG"
echo "Namespace: $NAMESPACE"

# Pre-deployment checks
echo "Running pre-deployment checks..."

# Check cluster connectivity
kubectl cluster-info || { echo "Cannot connect to cluster"; exit 1; }

# Check namespace exists
kubectl get namespace $NAMESPACE || { echo "Namespace $NAMESPACE does not exist"; exit 1; }

# Check required secrets
kubectl get secret cba-secrets -n $NAMESPACE || { echo "Required secrets not found"; exit 1; }

# Backup current deployment
echo "Creating backup of current deployment..."
kubectl get deployment cba-deployment -n $NAMESPACE -o yaml > backup-deployment-$(date +%Y%m%d_%H%M%S).yaml

# Update deployment
echo "Updating deployment..."
kubectl set image deployment/cba-deployment cba=$REGISTRY/$IMAGE_NAME:$IMAGE_TAG -n $NAMESPACE

# Wait for rollout
echo "Waiting for rollout to complete..."
kubectl rollout status deployment/cba-deployment -n $NAMESPACE --timeout=600s

# Verify deployment
echo "Verifying deployment..."
kubectl get pods -n $NAMESPACE -l app=cba

# Health check
echo "Running health checks..."
SERVICE_URL=$(kubectl get ingress cba-ingress -n $NAMESPACE -o jsonpath='{.spec.rules[0].host}')

for i in {1..30}; do
    if curl -f https://$SERVICE_URL/health; then
        echo "Health check passed"
        break
    fi
    echo "Health check failed, retrying in 10 seconds..."
    sleep 10
done

# Run smoke tests
echo "Running smoke tests..."
curl -f https://$SERVICE_URL/api/version
curl -f https://$SERVICE_URL/metrics

echo "Deployment completed successfully!"
echo "Application is available at: https://$SERVICE_URL"
```

## Troubleshooting

### Common Issues

#### 1. Container Won't Start

```bash
# Check container logs
docker logs cba-app

# Check resource usage
docker stats cba-app

# Check container configuration
docker inspect cba-app

# Common fixes:
# - Check file permissions
# - Verify configuration file syntax
# - Ensure all required environment variables are set
# - Check available disk space
```

#### 2. Database Connection Issues

```bash
# Test database connectivity
psql -h postgres -U cba_user -d cba_production -c "SELECT 1;"

# Check database logs
docker logs cba-postgres

# Verify network connectivity
docker exec cba-app ping postgres

# Common fixes:
# - Check database credentials
# - Verify network configuration
# - Check database server status
# - Review connection pool settings
```

#### 3. High Memory Usage

```bash
# Monitor memory usage
docker stats --no-stream

# Check for memory leaks
valgrind --tool=memcheck --leak-check=full ./cba-server

# Analyze heap usage
gdb -batch -ex "set logging on" -ex "set logging file heap.log" -ex "run" -ex "info proc mappings" -ex "quit" ./cba-server

# Common fixes:
# - Increase container memory limits
# - Optimize application memory usage
# - Check for memory leaks
# - Review garbage collection settings
```

#### 4. Performance Issues

```bash
# Monitor system resources
top -p $(pgrep cba-server)

# Check I/O performance
iostat -x 1

# Network performance
netstat -i

# Application profiling
perf record -g ./cba-server
perf report

# Common fixes:
# - Optimize database queries
# - Increase worker threads
# - Enable connection pooling
# - Use caching strategies
```

#### 5. SSL/TLS Issues

```bash
# Test SSL certificate
openssl s_client -connect api.yourdomain.com:443 -servername api.yourdomain.com

# Check certificate expiration
openssl x509 -in /etc/ssl/certs/cba/tls.crt -text -noout | grep "Not After"

# Verify certificate chain
openssl verify -CAfile /etc/ssl/certs/ca-certificates.crt /etc/ssl/certs/cba/tls.crt

# Common fixes:
# - Renew expired certificates
# - Check certificate chain
# - Verify domain name matches
# - Update cipher suites
```

### Monitoring and Alerting

#### Key Metrics to Monitor

1. **Application Metrics**
   - Request rate and response time
   - Error rate and status codes
   - Active connections
   - Queue lengths

2. **System Metrics**
   - CPU and memory usage
   - Disk I/O and space
   - Network throughput
   - Load average

3. **Database Metrics**
   - Connection pool usage
   - Query performance
   - Lock contention
   - Replication lag

4. **Infrastructure Metrics**
   - Container health
   - Kubernetes pod status
   - Load balancer health
   - SSL certificate expiration

#### Log Analysis

```bash
# Centralized logging with ELK stack
# Elasticsearch query examples

# Find errors in the last hour
GET /cba-logs-*/_search
{
  "query": {
    "bool": {
      "must": [
        { "match": { "level": "ERROR" } },
        { "range": { "@timestamp": { "gte": "now-1h" } } }
      ]
    }
  }
}

# Find slow requests
GET /cba-logs-*/_search
{
  "query": {
    "bool": {
      "must": [
        { "exists": { "field": "response_time" } },
        { "range": { "response_time": { "gte": 1000 } } }
      ]
    }
  },
  "sort": [ { "response_time": { "order": "desc" } } ]
}
```

### Disaster Recovery

#### Recovery Time Objectives (RTO)

- **Critical Services**: 15 minutes
- **Standard Services**: 1 hour
- **Non-critical Services**: 4 hours

#### Recovery Point Objectives (RPO)

- **Database**: 5 minutes (continuous replication)
- **Application Data**: 1 hour (hourly backups)
- **Configuration**: 24 hours (daily backups)

#### Disaster Recovery Procedures

```bash
#!/bin/bash
# scripts/disaster-recovery.sh

set -e

DR_REGION="us-west-2"
PRIMARY_REGION="us-east-1"
BACKUP_BUCKET="cba-dr-backups"

echo "Starting disaster recovery procedure..."

# 1. Assess damage
echo "Assessing primary region status..."
if kubectl cluster-info --context=primary-cluster; then
    echo "Primary cluster is accessible"
    CLUSTER_STATUS="healthy"
else
    echo "Primary cluster is not accessible"
    CLUSTER_STATUS="failed"
fi

# 2. Activate DR site
if [ "$CLUSTER_STATUS" = "failed" ]; then
    echo "Activating disaster recovery site..."
    
    # Switch to DR cluster
    kubectl config use-context dr-cluster
    
    # Restore from latest backup
    LATEST_BACKUP=$(aws s3 ls s3://$BACKUP_BUCKET/database/ | sort | tail -n 1 | awk '{print $4}')
    echo "Restoring from backup: $LATEST_BACKUP"
    
    # Download and restore database
    aws s3 cp s3://$BACKUP_BUCKET/database/$LATEST_BACKUP /tmp/
    gunzip -c /tmp/$LATEST_BACKUP | psql -h postgres-dr -U cba_user -d cba_production
    
    # Deploy application
    kubectl apply -f k8s/dr/ -n cba-production
    
    # Update DNS to point to DR site
    aws route53 change-resource-record-sets --hosted-zone-id Z123456789 --change-batch file://dns-failover.json
    
    echo "Disaster recovery site activated"
fi

# 3. Verify DR site
echo "Verifying disaster recovery site..."
DR_URL="https://api-dr.yourdomain.com"
curl -f $DR_URL/health || { echo "DR site health check failed"; exit 1; }

echo "Disaster recovery completed successfully"
echo "Application is now running on DR site: $DR_URL"
```

## Conclusion

This production deployment guide provides comprehensive instructions for deploying CoreBaseApplication in a production environment. Key points to remember:

1. **Security First**: Always implement security hardening measures
2. **Monitoring**: Set up comprehensive monitoring and alerting
3. **Automation**: Use CI/CD pipelines for consistent deployments
4. **Backup Strategy**: Implement regular backups and test recovery procedures
5. **Performance**: Monitor and optimize application performance
6. **Documentation**: Keep deployment procedures up to date

For additional support, refer to:
- [Architecture Overview](../architecture/overview.md)
- [Security Guidelines](../development/security.md)
- [Performance Optimization](../development/performance.md)
- [API Reference](../api/reference.md)

---

**Next Steps:**
1. Review and customize configurations for your environment
2. Set up monitoring and alerting systems
3. Test backup and recovery procedures
4. Implement CI/CD pipeline
5. Conduct security audit
6. Plan disaster recovery strategy