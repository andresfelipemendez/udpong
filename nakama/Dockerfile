ARG NAKAMA_VERSION=3.24.2

FROM registry.heroiclabs.com/heroiclabs/nakama-pluginbuilder:${NAKAMA_VERSION} AS builder

ENV GO111MODULE=on
ENV CGO_ENABLED=1

WORKDIR /backend
COPY modules/go/ .

RUN go mod tidy && go build --trimpath --buildmode=plugin -o ./pong.so

FROM registry.heroiclabs.com/heroiclabs/nakama:${NAKAMA_VERSION}

COPY --from=builder /backend/pong.so /nakama/data/modules/
