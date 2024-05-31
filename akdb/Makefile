# KalashnikovDB Docker MakeFile

docker-install:
	docker build -t akdb .

docker-win:
	docker run -it -v "%cd%":/usr/akdb/ --privileged akdb

docker-mac:
	docker run -it -v $(CURDIR):/usr/akdb/ --privileged akdb