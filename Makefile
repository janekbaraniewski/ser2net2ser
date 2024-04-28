BUILD_DIR = build
REPO = ghcr.io/janekbaraniewski/ser2net2ser
VERSION ?= latest
COMMIT_HASH ?= $(shell git rev-parse --short HEAD)

clean:
	@rm -rf ${BUILD_DIR}/*

build: clean
	@cmake -S . -B build
	@cmake --build build

test: build
	@cd build && ctest

build-images:
	@docker build . -f Dockerfile.server -t $(REPO)/server:$(COMMIT_HASH)
	@docker build . -f Dockerfile.client -t $(REPO)/client:$(COMMIT_HASH)

tag-images:
	@docker tag $(REPO)/server:$(COMMIT_HASH) $(REPO)/server:$(VERSION)
	@docker tag $(REPO)/client:$(COMMIT_HASH) $(REPO)/client:$(VERSION)

push-images: tag-images
	@docker push $(REPO)/server:$(COMMIT_HASH)
	@docker push $(REPO)/server:$(VERSION)
	@docker push $(REPO)/client:$(COMMIT_HASH)
	@docker push $(REPO)/client:$(VERSION)

help:
	@echo "Makefile commands:"
	@echo "  clean          - Remove build directory."
	@echo "  build          - Build the project using cmake."
	@echo "  test           - Run tests after building."
	@echo "  build-images   - Build Docker images for both server and client."
	@echo "  tag-images     - Tag Docker images with the latest or specified version."
	@echo "  push-images    - Push Docker images to the registry."
	@echo "  help           - Show this help message."

.PHONY: clean build test build-images tag-images push-images help
