# KalashnikovDB Docker MakeFile

docker-install:
	docker build -t akdb .
docker-compose:
	docker-compose up -d --build
	docker-compose exec akdb bash
	make docker-compose-stop

docker-compose-stop:
	docker-compose down

docker-run:
	docker run -it -v $(CURDIR):/usr/akdb/ --privileged akdb


docker-win:
	docker run -it -v "%cd%":/usr/akdb/ --privileged akdb

docker-mac:
	docker run -it -v $(CURDIR):/usr/akdb/ --privileged akdb