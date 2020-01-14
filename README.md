I'm using git lfs to store the compressed docker image, so the **download may take awhile.**


# Challenge 1


## Running

I provided the docker image as a gzipped tarball in the `chall1` directory.

You'll need to load it and then run a container that maps port 8080.
```shell
    docker load --input paxos-chall1.tar.gz
    docker run -p 8080:8080 wimmeldj/paxos-chall1:latest
```
You can then make the standard requests to `localhost:8080/...`

You can run the tests in [tests/chall1test](./tests/chall1test) after starting the docker image.

If you don't want to use the docker image and you have npm and node installed, you can just run `npm
start`.


## Approach Explanation and Question Answers

There isn't much to say about this approach since it's just a simple node http server. The
application is just storing the message hashes and the messages in memory, so if you restart the
service, these obviously won't persist. Of course, if this were a real application, you would at
least have a database to store these in with different implementations of the storage varying by the
scale of the application, goals, etc.


### Scaling Question

How we scale the microservice would depend a lot on how it's being used. If this isn't just an
internal application, you would want to use api keys to restrict your service to actual users. This
could also be used to symmetrically encrypt client requests.

Probably the biggest bottleneck/point of failure is the database server. We would most likely be
using a NoSQL database like MongoDB since this application is practically made for key-value pair
storage. Because of this, we could use sharding to easily horizontally scale the database. Since
we're already storing messages by their sha256 hash, this would be a perfect shard key and we
wouldn't need to store any extra values to efficiently distribute the database. If users are
frequently requesting the same message, you might also want to add an in memory database like redis
to cache content.

Another potential point of failure due to an increased user base would be the server the api
application is running on. To decrease the overall load on the api, you would want to horizontally
scale this to multiple machines and have a load balancer route requests to these machines.

You could also use a CDN to decrease load on both the database servers and api servers. AWS
cloudfront would be one option for this. And if you're using a pull cdn, you might be able to avoid
having an in-memory database altogether.

One other thing that comes to mind is the size of the messages we're allowing users to upload. Is
this api just for storing human language messages or are users base64 encoding collections of photos
and uploading this as a "message." If the latter is the case, you might want to implement a
different storage solution for large messages. For instance, we could set a size limit on messages
we store in the NoSQL databases. For messages larger than this, we could use a distributed object
storage service like AWS S3 to store the actual data. In the database, we would instead store an s3
uri, indicating that we would need to retrieve the data from some other place.


### Deployment Question

First of all, you should write a docker compose file so that you can specify exact procedures for
deploying the application. These procedures can then be validated and version controlled.

You would want to implement some scheme for API versioning. This could be as simple as a version
number prefacing the url. The reason for doing this is so that potentially application breaking
changes do not affect end users. You could tie the api's version into the version of the docker
image. So that you can easily tell what version of the api an image is capable of supporting.

Lastly, you could implement a CI/CD pipeline to handle changes to the codebase and automatically
deploy them. For instance, you could set up jenkins to deploy to production any changes pushed to
the master git branch. You might also have jenkins deploy changes to a sandbox branch, which would
be linked to an application running on an internal network for testing new features.


# Challenge 2


## Compiling/Running

You shouldn't get *any* errors when running the compilation below. Do this in the `chall2`
directory.
```shell
    gcc -std=gnu11 -Wall -Wextra -pedantic -o find-pair *.c
```
If you can't compile, though, I provided the binary "find-pair" for the `x86_64-pc-linux-gnu` gcc
target which you should be able to run on any modern 64 bit linux virtual machines.

You can see a usage message if you run `find-pair -h`.

Here are some examples of running. The `-n` option was added to complete bonus question A. If you do
`-n 3`, this will find the optimum triplet. You can do `-n 2` as well, but since 2 is default, this
is the same as not providing the `-n` argument.
```shell
    ./find-pair prices.txt 10000 #=> Find optimum pair from prices.txt file with giftcard of 10,000 cents
    ./find-pair -n 2 prices.txt 10000 #=> same
    ./find-pair -n 3 prices.txt 10000 #=> Same again, but find optimum triplet
```

Basic tests are provided in [tests/chall2test](./tests/chall2test)

## Approach Explanation and Question Answers

Obviously, implementing this in C is kind of unnecessary since this could probably be done in 50
lines of python. But I thought it would be more fun. One part that might be slightly confusing is I
implemented a basic dynamically resizing array to store the file data. You can read the header
comments in [chall2_lib.h](./chall2/chall2_lib.h) to see how it's used.

The algorithm is simple and just uses the double pointer narrowing method. Where we have a left most
pointer (min value) and right most pointer (max value).

1.  If the sum equals our target, we immediately break.
2.  When the sum of these two is greater than our target, we narrow the window by decrementing the
    right pointer.
3.  Otherwise, we increment the left pointer. Updating our current best sum if necessary.
4.  If the left pointer and right pointer equal each other, we either return our current best sum or
    "not possible" if it was never set.

-   **Time complexity of the algorithm is O(n) since the data was already sorted.**
-   **Space complexity is O(n) since we store only one copy of all the data read.**


## Bonus


### A) Three friends

Allowing for three gifts was implemented in the `three_friends` function in
[chall2.c](./chall2/chall2.c) The basic idea is that we can still use the algorithm above on a
suffix of the prices array, while maintaining a left-most pointer that sweeps over all elements of
the array starting at 0. When we call the `max_pair_under` function, however, our target isn't
actually the input giftcard amount, but rather the difference between the giftcard amount and the
value at the left most pointer `giftamt - prices[i]`.

-   **Time complexity is O(n<sup>2</sup>) since we call an O(n) function n times**
-   **Space complexity is still O(n)**


### B) Can't Load File in Memory

This would actually be relatively simple to implement for the `two_friends` function and a little
bit more tricky for `three_friends`. Since we can't load the whole file into memory, we would want
to get a chunk at the head of the file and a chunk at the tail of the file, saving a file pointer
for both of these locations after retrieving them. When either pointer (left or right) gets to the
end of a chunk, free the memory allocated for the chunk and retrieve another chunk from either the
file pointer at the head (left) or the tail (right).

You would continue this process until when retrieving a chunk, the head and tail file pointers are
about to cross. At this point, you just return the chunk between the head and tail file pointers (to
avoid duplication of data and bad answers).

For three friends, it's trickier. You will need to retrieve a head chunk for the left most pointer
to traverse. And you will need to follow the algorithm above for the suffix array. You continue
retrieving chunks from the head file pointer until you reach the end of the file.
