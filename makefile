include env.sh

# 1. Subir código (Sincronización inteligente, no borra carpeta 'out' remota innecesariamente)
deploy:
	bash scripts/deploy/sync.sh $(REMOTE_USER) $(REMOTE_HOST) $(REMOTE_DIR)

# 2. Compilar en Avignon (Manda el comando ssh)
remote-build: deploy
	ssh $(REMOTE_USER)@$(REMOTE_HOST) "cd $(REMOTE_DIR) && sbatch --wait scripts/remote/build.sh"

# 3. Generar tests (Ejecuta tu python localmente para crear los .sh)
gen-tests:
	python3 scripts/generation/gen_tests.py

# 4. Enviar TODOS los tests generados a la cola de Avignon
remote-run-all: deploy
	ssh $(REMOTE_USER)@$(REMOTE_HOST) "cd $(REMOTE_DIR) && find tests_de_config tests_de_escenas -name '*.sh' -exec sbatch {} \;"

# 5. Descargar resultados (CSV y PPMs de salida)
fetch:
	rsync -avz --exclude '*.o' --exclude 'cmake*' $(REMOTE_USER)@$(REMOTE_HOST):$(REMOTE_DIR)/resultados_consolidados.csv ./out/
	rsync -avz $(REMOTE_USER)@$(REMOTE_HOST):$(REMOTE_DIR)/*.ppm ./out/remote_imgs/

# Atajo completo: Sube -> Compila -> Lanza Tests
all-remote: remote-build remote-run-all