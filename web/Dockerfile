#from openalpr:python3
#workdir /srv/openalpr/web/server
#copy . .
#run pip3 install -r req.txt
#
#EXPOSE 5000
#CMD ["gunicorn", "-b", ":5000", "main:app"]

# Build step #1: build the React front end
FROM node:16-alpine as build-step
WORKDIR /app
ENV PATH /app/node_modules/.bin:$PATH
RUN mkdir src
RUN mkdir public
COPY ./client/package.json .
COPY ./client/package-lock.json .
RUN npm install

COPY ./client/src ./src
COPY ./client/public ./public
RUN npm run build

# Build step #2: build the API with the client as static files
FROM openalpr:python3
WORKDIR /app/build
COPY --from=build-step /app/build .

WORKDIR /app
COPY ./server/req.txt .
RUN pip3 install -r req.txt
COPY ./server .

EXPOSE 3000
CMD ["python3", "main.py"]
#CMD ["gunicorn", "-b", ":3000", "main:app"]