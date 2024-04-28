BUILD_DIR = build
REPO = ghcr.io/janekbaraniewski/ser2net2ser
VERSION ?= latest
COMMIT_HASH ?= $(shell git rev-parse --short HEAD)
CMAKE_FLAGS ?= -DCMAKE_BUILD_TYPE=Release

help:  ## Show this help message
	@echo "Usage: make [target]"
	@echo ""
	@echo "Available targets:"
	@grep -E '^[a-zA-Z_\-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  %-30s %s\n", $$1, $$2}'

clean: ## Clean build directory
	@rm -rf ${BUILD_DIR}/*

build: ## Build with cmake
build: clean
	@cmake -S . -B build $(CMAKE_FLAGS)
	@cmake --build build

test: ## Build and test
test: build
	@cd build && ctest

build-images: ## Build container images
build-images: build-server-image build-client-image

build-server-image:
	@docker build . -f Dockerfile.server -t $(REPO)-server:$(COMMIT_HASH)

build-client-image:
	@docker build . -f Dockerfile.client -t $(REPO)-client:$(COMMIT_HASH)

tag-images: ## Tag container images
tag-images: tag-server-image tag-client-image

tag-server-image:
	@docker tag $(REPO)/server:$(COMMIT_HASH) $(REPO)-server:$(VERSION)

tag-client-image:
	@docker tag $(REPO)/client:$(COMMIT_HASH) $(REPO)-client:$(VERSION)


push-images: ## Push container images to registry
push-images: tag-images push-server-images push-client-images

push-server-images:
	@docker push $(REPO)-server:$(COMMIT_HASH)
	@docker push $(REPO)-server:$(VERSION)

push-client-images:
	@docker push $(REPO)-client:$(COMMIT_HASH)
	@docker push $(REPO)-client:$(VERSION)

.PHONY: clean build test build-images build-server-image build-client-image tag-images tag-client-image tag-server-image push-images push-client-images push-server-images help
