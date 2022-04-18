use std::io::*;
use std::net::*;
use std::sync::*;
use std::env;
use std::os::unix::io::RawFd;
use std::os::unix::io::AsRawFd;

struct Client {
    stream: Arc<Mutex<TcpStream>>,
    fd: RawFd
}

type Clients = Arc<Mutex<Vec<Arc<Mutex<TcpStream>>>>>;

impl Client {

    /// send to one provided TCP stream 
    fn sendln_to_stream(msg: &str, stream: &Arc<Mutex<TcpStream>>) -> std::io::Result<usize> {
        let mut stream = stream.lock().unwrap();
		println!("Sending to client {}", stream.as_raw_fd());

        // TODO 6: Send the message to 'stream'
        Ok(0)
    }
    
    /// send to all other clients except ourselves
    fn broadcastln(&mut self, msg: &str, clients: &Clients) -> std::io::Result<()> {
        clients.lock().unwrap().iter()
            .filter(|c| !Arc::ptr_eq(&self.stream, c))
            .for_each(|c| {
                match Client::sendln_to_stream(msg, &c) {
                    Err(E) => {println!("Failed to send to client. Error: {}", E)},
                    Ok(_) => {}
                }
            });
        Ok(())
    }

    // serve requests for this client
    fn run(&mut self, clients: &Clients) -> std::io::Result<()> {

        // TODO 5a: Create a BufReader over the stream
        // The BufReader is used to parse the input stream and split it by newlines
        // Replace
        let mut input = {

            // Step 1: Acquire the mutex under self.stream and bind 'stream' to the reference under it
            // let stream = ?;

            // Step 2: Create a BufReader using the constructor of BufReader passing it a
            // clone of the reference which is under self.stream.
            // You can create the clone by using the try_clone method of Arc
            // Replace std::io::empty with the new argument
            BufReader::new(std::io::empty())
        };

        loop {
            let mut buf = String::new();
            match input.read_line(&mut buf) {
                Ok(0) => {
                    return Ok(());
                },
                Err(err) => {
                    return Err(err)
                },
                Ok(_size) => {
                    self.broadcastln(&format!("Client {}: {}", self.fd, buf), clients)?;
                    // TODO 5b: Send a message back to this client to inform it that the broadcast has been
                    // done
                }
            }
        }
    }
}

fn main() {

    let args: Vec<String> = env::args().collect();
    if args.len() != 2 {
        println!("Invalid usage.\nProvide port as the first argument");
        return;
    }

    // Set up data structures and port
    let port = args[1].parse::<i32>().unwrap();
    let clients: Clients = Arc::new(Mutex::new(Vec::new()));

    // TODO (1, 2): Create listener to listen on port
    let listener: TcpListener;
    println!("Listening on {}", port);


    // TODO 3, 4: Accept incoming connections
    // Replace 'streams' below with the code that uses the listeners and returns a sequence of streams
    // Delete 'streams' from the code after you're done, it's only purpose was to enable compilation of the template code.
    let streams: Vec<std::result::Result<std::net::TcpStream, std::io::Error>> = Vec::new();
    for stream in streams {

        if let Ok(stream) = stream {

            // A connection has been accepted now
            let fd = stream.as_raw_fd();
            println!("Server accepted connection from client with fd {}", fd);
            let stream = Arc::new(Mutex::new(stream));

            // Add connection to the clients list
            clients.lock().unwrap().push(stream.clone());

            // Start a thread to serve requests for that client
            let clientsclone = clients.clone();       
            std::thread::spawn(move || {

                // Serve requests
                let mut c = Client { stream: stream.clone(), fd: fd };
                match c.run(&clientsclone) {
                    Ok(()) => { println!("Session concluded successfully."); },
                    Err(e) => { println!("Session ended with error: {:?}",e); }
                }

                // Remove connection from the clients list once done
                let mut clients = clientsclone.lock().unwrap();
                let index = clients.iter().position(move |x| Arc::ptr_eq(&stream, x)).unwrap();
                clients.remove(index);
            });

        }
    }
}
