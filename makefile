include env.sh

PASSFILE=.password

# 1. Subir código
deploy:
	@bash scripts/deploy/sync.sh $(REMOTE_USER) $(REMOTE_HOST) $(REMOTE_DIR)

# 2. Compilar
remote-build: deploy
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
	@echo ">>> Descargando directorio de imágenes..."
	@mkdir -p logs
	@sshpass -f $(PASSFILE) scp -r -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST):$(REMOTE_DIR)/logs/images ./logs/ || true

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