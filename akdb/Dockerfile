FROM alpine:3.19

# Build argument for LaTeX installation
ARG BUILD_DOCS=false

# Dependencies
RUN apk add --no-cache \
coreutils \
build-base \
swig \
coreutils \
make \
git \
curl \
dos2unix \
libffi-dev \
openssl-dev \
python3 \
python3-dev \
py3-pip \
ca-certificates \
tzdata

# Install LaTeX tools
RUN if [ "$BUILD_DOCS" = "true" ]; then \
apk add --no-cache \
texlive \
texlive-xetex \
texlive-luatex \
doxygen; \
fi

# Python packages
RUN pip3 install --no-cache-dir --break-system-packages \
--no-compile \
--disable-pip-version-check \
virtualenv \
paramiko \
pyparsing==2.4.7 \
configparser \
langdetect && \
pip3 cache purge && \
find /usr -type d -name '__pycache__' -exec rm -rf {} + 2>/dev/null || true && \
find /usr -name '*.pyc' -delete 2>/dev/null || true && \
find /usr -name '*.pyo' -delete 2>/dev/null || true && \
find /usr -type d -name '*.dist-info' -exec rm -rf {} + 2>/dev/null || true && \
rm -rf /root/.cache /tmp/* /var/tmp/*

WORKDIR /usr/akdb
VOLUME ["/usr/akdb"]

