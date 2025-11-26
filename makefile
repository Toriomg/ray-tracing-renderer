include env.sh

PASSFILE=.password

# 1. Subir código (Sincronización inteligente, no borra carpeta 'out' remota innecesariamente)
deploy:
	bash scripts/deploy/sync.sh $(REMOTE_USER) $(REMOTE_HOST) $(REMOTE_DIR)

# 2. Compilar en Avignon (Manda el comando ssh)
remote-build: deploy
	@echo ">>> Enviando compilación a la cola..."
	sshpass -f $(PASSFILE) ssh -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST) "cd $(REMOTE_DIR) && sbatch --wait scripts/remote/build.sh"


# 3. Generar tests (Ejecuta tu python localmente para crear los .sh)
#gen-tests:
#	python3 scripts/generation/gen_tests.py

run-jd: deploy
	@echo ">>> Enviando run-test-jd.sh a la cola de Slurm..."
	sshpass -f $(PASSFILE) ssh -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST) "cd $(REMOTE_DIR) && sbatch -p stan scripts/remote/run-test-jd.sh"

tail-jd:
	@echo ">>> Leyendo log del trabajo $(ID)..."
	sshpass -f $(PASSFILE) ssh -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST) "tail -n 20 -f $(REMOTE_DIR)/logs/run-test-jd.txt"

all-jd: run-jd tail-jd

#remote-run-all: deploy 
# ssh $(REMOTE_USER)@$(REMOTE_HOST) "cd $(REMOTE_DIR) && find tests_de_config tests_de_escenas -name '*.sh' -exec sbatch {} \;"



# --- DESCARGA DE RESULTADOS ---

# 1. Descargar solo imágenes (.ppm)
fetch-ppm:
	@echo ">>> Creando carpeta logs y descargando imágenes..."
	@mkdir -p logs
	-sshpass -f $(PASSFILE) scp -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST):$(REMOTE_DIR)/*.ppm ./logs/img

# 2. Descargar logs de texto (.txt y .out de slurm)
fetch-txt:
	@echo ">>> Descargando logs de texto..."
	@mkdir -p logs
	# Descargamos .txt y también .out (que son los logs de Slurm)
	-sshpass -f $(PASSFILE) scp -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST):"$(REMOTE_DIR)/logs/*.txt" ./logs/txt
	-sshpass -f $(PASSFILE) scp -o StrictHostKeyChecking=no $(REMOTE_USER)@$(REMOTE_HOST):"$(REMOTE_DIR)/logs/*.out" ./logs/txt

# 3. Descargar TODO (Junta los dos anteriores)
fetch-all: fetch-ppm fetch-txt
	@echo ">>> ✅ Descarga completa en ./logs/"


