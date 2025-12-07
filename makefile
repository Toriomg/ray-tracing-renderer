include env.sh

PASSFILE=.password

# Para los scripts sweep
SSH_CMD=sshpass -f $(PASSFILE) ssh -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST)
SCP_PREFIX=sshpass -f $(PASSFILE) scp -o StrictHostKeyChecking=no

# 1. Subir código
deploy:
	@bash scripts/deploy/sync.sh $(REMOTE_USER) $(REMOTE_HOST) $(REMOTE_DIR)

# 2. Compilar
remote-build: deploy
	@echo ">>> Asegurando permisos de ejecución en scripts/remote/..."
	@sshpass -f $(PASSFILE) ssh -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST) "cd $(REMOTE_DIR) && chmod +x scripts/remote/*.sh"
	@echo ">>> Enviando compilación a la cola..."
	@sshpass -f $(PASSFILE) ssh -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST) "cd $(REMOTE_DIR) && sbatch --wait scripts/remote/build.sh"

run-jd: deploy
	@echo ">>> Limpiando y preparando directorios..."
	@sshpass -f $(PASSFILE) ssh -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST) "rm -rf $(REMOTE_DIR)/logs/txt $(REMOTE_DIR)/logs/images && mkdir -p $(REMOTE_DIR)/logs/txt $(REMOTE_DIR)/logs/images"
	
	@echo ">>> Enviando run-test-jd.sh a la cola de Slurm..."
	@sshpass -f $(PASSFILE) ssh -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST) "cd $(REMOTE_DIR) && sbatch -p stan scripts/remote/run-test-jd.sh"

tail-jd:
	@echo ">>> Leyendo log (esperando creación)..."
	@sshpass -f $(PASSFILE) ssh -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST) "tail -n 20 -F $(REMOTE_DIR)/logs/txt/run-test-jd.out"

all-jd: run-jd tail-jd


# --- DESCARGA DE RESULTADOS ---

fetch-ppm:
	@echo ">>> Creando carpeta logs y descargando imágenes..."
	@mkdir -p logs/img
	-sshpass -f $(PASSFILE) scp -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST):$(REMOTE_DIR)/*.ppm ./logs/img/

fetch-txt:
	@echo ">>> Descargando directorio de logs de texto..."
	@mkdir -p logs
	@sshpass -f $(PASSFILE) scp -r -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST):$(REMOTE_DIR)/logs/txt ./logs/ || true

compare:
	@for img in $$(ls logs/images/out*.ppm 2>/dev/null | grep -E 'out[0-9]+\.ppm$$'); do \
		ID=$$(echo $$img | grep -o -E '[0-9]+'); \
		REF="compare-images/s$${ID}-par.ppm"; \
		if [ -f "$$REF" ]; then \
			echo "Checking Scene $$ID:"; \
			python3 scripts/analysis/compare.py "$$REF" "$$img"; \
		else \
			echo "Checking Scene $$ID: ... Reference not found (SKIPPING)"; \
		fi; \
	done

pipeline:
	@$(MAKE) compare 2>&1 | python3 scripts/analysis/validate_pipeline.py --log-file logs/txt/run-test-jd.out

fetch-all: fetch-ppm fetch-txt pipeline

run-jd-wait: deploy
	@echo ">>> Limpiando y preparando directorios..."
	@sshpass -f $(PASSFILE) ssh -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST) "rm -rf $(REMOTE_DIR)/logs/txt $(REMOTE_DIR)/logs/images && mkdir -p $(REMOTE_DIR)/logs/txt $(REMOTE_DIR)/logs/images"
	
	@echo ">>> Enviando trabajo y esperando a que termine..."
	@sshpass -f $(PASSFILE) ssh -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST) "cd $(REMOTE_DIR) && sbatch --wait -p stan scripts/remote/run-test-jd.sh"

auto-jd: remote-build run-jd-wait fetch-all
	@echo "AUTO JD COMPLETADO"

# Scripts sweep (Flujo tradicional: config óptima → escalabilidad)
sweep-opt:
	$(SSH_CMD) "cd $(REMOTE_DIR) && sbatch scripts/remote/sweep_optimization.sh"

sweep-scale:
	$(SSH_CMD) "cd $(REMOTE_DIR) && sbatch scripts/remote/sweep_scalability.sh $(PART) $(GRAIN) $(START) $(END) $(STEP)"

# Nuevos scripts (Flujo metodológico: hilos primero → granularidad después)
sweep-threads-first:
	@echo ">>> PASO 1: Explorando número óptimo de hilos (28, 56, 112, 120)..."
	$(SSH_CMD) "cd $(REMOTE_DIR) && sbatch scripts/remote/sweep_threads_first.sh"

sweep-grain:
	@echo ">>> PASO 2: Explorando granularidad óptima con $(THREADS) hilos fijos..."
	$(SSH_CMD) "cd $(REMOTE_DIR) && sbatch scripts/remote/sweep_grain.sh $(THREADS)"

fetch-results:
	@echo ">>> Descargando resultados CSV..."
	sshpass -f $(PASSFILE) scp -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST):$(REMOTE_DIR)/logs/*.csv ./logs/

# Scripts sweep2
run-custom:
	@echo ">>> Enviando prueba personalizada a la cola..."
	$(SSH_CMD) "cd $(REMOTE_DIR) && sbatch scripts/remote/test_custom.sh"

tail-custom:
	$(SSH_CMD) "tail -f \`ls -t $(REMOTE_DIR)/logs/custom_*.out | head -n1\`"


# --- VALIDACIÓN Y COMPARACIÓN ---

# Define dónde guardaste las referencias del profesor
REF_DIR=res/references_par

# Compara la salida de run-custom con la referencia oficial (Escenario 5)
compare-custom: fetch-ppm
	@echo ">>> 🔍 Comparando out_custom.ppm con la referencia s5-par.ppm..."
	# Asegúrate de que tienes la referencia en res/references_par/
	python3 scripts/analysis/compare.py $(REF_DIR)/s5-par.ppm logs/img/out_custom.ppm
	@echo ">>> ✅ Si hay diferencias, se ha generado una imagen en logs/img/out_custom.ppmdiferencias.ppm"