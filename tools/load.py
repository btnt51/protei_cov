import concurrent.futures
import http.client
import argparse

def send_request(host, port, path):
    conn = http.client.HTTPConnection(host, port)
    conn.request("GET", path)
    response = conn.getresponse()
    data = response.read()
    print(f"Response from {host}:{port}{path}:")
    print(f"Response Code: {response.status}")
    print("Response Data:")
    print(data.decode())
    conn.close()

def send_batch(nums, host, port):
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = [executor.submit(send_request, host, port, f"/phone={num}") for num in nums]

    # Wait for all futures to complete
    concurrent.futures.wait(futures)

def interactive_mode(nums, host, port, batch_size):
    while True:
        user_input = input("Do you want to send a new batch of messages? (yes/no): ").lower()
        if user_input != 'yes':
            break

        # Send messages in batches
        for i in range(0, len(nums), batch_size):
            send_batch(nums[i:i+batch_size], host, port)


def automatic_mode(nums, host, port, num_connections):
    with concurrent.futures.ThreadPoolExecutor(max_workers=num_connections) as executor:
        futures = [executor.submit(send_request, host, port, f"/phone={num}") for num in nums]

    # Wait for all futures to complete
    concurrent.futures.wait(futures)


def main():
    parser = argparse.ArgumentParser(description='Concurrent HTTP requests script')
    parser.add_argument('--num_connections', type=int, default=None, help='Number of concurrent connections')
    parser.add_argument('--automatic', action='store_true', help='Enable automatic mode')
    args = parser.parse_args()

    nums = [str(x) for x in range(2500)]
    host = "localhost"
    port = 8080

    if args.automatic:
        automatic_mode(nums, host, port, args.num_connections)
    else:
        interactive_mode(nums, host, port, args.num_connections)

if __name__ == "__main__":
    main()
