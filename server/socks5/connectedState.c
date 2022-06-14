#include "./../../include/connectedState.h"

void connected_init(struct socks5 * currClient) {

	// Init connected for the client
	connected *d = &currClient->client.st_connected;

	d->fd = currClient->client_socket;
	d->r = currClient->bufferFromClient;
	d->w = currClient->bufferFromRemote;
	d->interest = OP_READ | OP_WRITE;
	d->other_connected = &currClient->remote.st_connected;

	// Init connected for origin
	d = &currClient->remote.st_connected;

	d->fd = currClient->remote_socket;
	d->r = currClient->bufferFromRemote;
	d->w = currClient->bufferFromClient;
	d->interest = OP_READ | OP_WRITE;
	d->other_connected = &currClient->client.st_connected;
}

static fd_interest
connected_determine_interests(fd_selector s, connected *d) {
	fd_interest interest = OP_NOOP;

	if ((d->interest & OP_READ) && buffer_can_write(d->r))
    {
        // Add the interest to read
        interest |= OP_READ;
    }

	if ((d->interest & OP_WRITE) && buffer_can_read(d->w))
    {
        // Add the interest to write
        interest |= OP_WRITE;
    }

	// Set the interests for the selector
    if (SELECTOR_SUCCESS != selector_set_interest(s, d->fd, interest))
    {
        printf("Could not set interest of %d for %d\n", interest, d->fd);
        abort();
    }
    return interest;
}

static connected * get_connected_ptr(struct selector_key *key)
{
    // Getting the connected struct for the client
	struct socks5 * currClient = (struct socks5 *)key->data;
    connected *d = &currClient->client.st_connected;

    // Checking if the selector fired is the client by comparing the fd
    if (d->fd != key->fd)
    {
        d = d->other_connected;
    }
    return d;
}

void write_connected_state(struct selector_key *key) {
	connected *d = get_connected_ptr(key);

	buffer *b = d->w;
	uint8_t *ptr;
    size_t count;
    ssize_t n;

	// Setting the buffer to read
    ptr = buffer_read_ptr(b, &count);

	n = send(key->fd, ptr, count, MSG_NOSIGNAL);

	if (n != -1) {
		buffer_read_adv(b, n);
	} else {
		// Closing the socket for writing
        shutdown(d->fd, SHUT_WR);
		// Removing the interest to write from this connected
        d->interest &= ~OP_WRITE;
        // If the other fd is still open
        if (d->other_connected->fd != -1)
        {
            // Closing the socket for reading
            shutdown(d->other_connected->fd, SHUT_RD);
            // Remove the interest for reading
            d->other_connected->interest &= ~OP_READ;
        }
	}

	// Determining the new interests for the selectors
    connected_determine_interests(key->s, d);
    connected_determine_interests(key->s, d->other_connected);
}

void read_connected_state(struct selector_key *key) {
	connected *d = get_connected_ptr(key);

	buffer *b = d->r;
	uint8_t *ptr;
	size_t count;
	ssize_t n;

	ptr = buffer_write_ptr(b, &count);

	n = recv(key->fd, ptr, count, 0);
	if (n > 0) {
		buffer_write_adv(b, n);
	} else {
		shutdown(d->fd, SHUT_RD);
		// Removing the interest to read from this connected
        d->interest &= ~OP_READ;
		// If the other fd is still open
        if (d->other_connected->fd != -1)
        {
            // Closing the socket for writing
            shutdown(d->other_connected->fd, SHUT_WR);
            // Remove the interest to write
            d->other_connected->interest &= ~OP_WRITE;
        }
	}

	// Determining the new interests for the selectors
    connected_determine_interests(key->s, d);
    connected_determine_interests(key->s, d->other_connected);

	if (d->interest == OP_NOOP)
    {
        socks5_done(key);
    }
}