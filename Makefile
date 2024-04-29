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
	@docker build . -f Dockerfile -t $(REPO):$(COMMIT_HASH)

tag-images: ## Tag container images
	@docker tag $(REPO):$(COMMIT_HASH) $(REPO):$(VERSION)


push-images: ## Push container images to registry
	@docker push $(REPO):$(COMMIT_HASH)
	@docker push $(REPO):$(VERSION)

.PHONY: clean build test build-images build-server-image build-client-image tag-images tag-client-image tag-server-image push-images push-client-images push-server-images help
