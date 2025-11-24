/* railway_booking_qr.c
   Railway Ticket Booker with:
    - Duplicate booking prevention
    - QR code generation (libqrencode if available; fallback ASCII otherwise)

   Compile (Linux with libqrencode installed):
     gcc railway_booking_qr.c -o railway_booking_qr -lqrencode

   Compile without libqrencode:
     gcc railway_booking_qr.c -o railway_booking_qr

   Notes:
    - On Debian/Ubuntu: sudo apt install libqrencode-dev
    - On Windows (MSYS2 / MinGW): install qrencode package or compile libqrencode and link.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* If libqrencode is available on your system, define HAVE_QRENCODE (or compile with -DHAVE_QRENCODE)
   and link with -lqrencode. The code will then produce a PBM image file with the QR.
*/
#ifdef HAVE_QRENCODE
#include <qrencode.h>
#endif

#define MAX_NAME 100
#define MAX_CLASS 20
#define BOOKINGS_FILE "bookings.dat"
#define MAX_TRAINS 5

typedef struct {
    int booking_id;
    char passenger_name[MAX_NAME];
    int age;
    char gender[10];
    int train_id;
    char travel_class[MAX_CLASS];
} Booking;

typedef struct Node {
    Booking b;
    struct Node *next;
} Node;

/* Predefined trains */
typedef struct {
    int id;
    char name[80];
    char from[50];
    char to[50];
    int total_seats;
} Train;

Train trains[MAX_TRAINS] = {
    {1, "Express A", "Mumbai", "Delhi", 100},
    {2, "Superfast B", "Kolkata", "Bangalore", 80},
    {3, "Intercity C", "Chennai", "Hyderabad", 60},
    {4, "Mail D", "Jaipur", "Lucknow", 50},
    {5, "Shatabdi E", "Ahmedabad", "Pune", 90}
};

Node *head = NULL;
int next_booking_id = 1;

/* utils */
void chomp(char *s) {
    size_t len = strlen(s);
    if (len == 0) return;
    if (s[len-1] == '\n') s[len-1] = '\0';
}
void strtolower(char *s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}
int equalstr_nospaces_case(const char *a, const char *b) {
    // compare trimmed, case-insensitive
    while (isspace((unsigned char)*a)) ++a;
    while (isspace((unsigned char)*b)) ++b;
    // copy to temp lower-case
    char ta[MAX_NAME], tb[MAX_NAME];
    size_t i=0, j=0;
    for (; *a && i+1<sizeof(ta); ++a) if (!isspace((unsigned char)*a)) ta[i++]= (char)tolower((unsigned char)*a);
    ta[i]=0;
    for (; *b && j+1<sizeof(tb); ++b) if (!isspace((unsigned char)*b)) tb[j++]= (char)tolower((unsigned char)*b);
    tb[j]=0;
    return strcmp(ta,tb)==0;
}

/* file persistence */
void load_bookings() {
    FILE *fp = fopen(BOOKINGS_FILE, "rb");
    if (!fp) return;
    Booking tmp;
    int maxid = 0;
    while (fread(&tmp, sizeof(Booking), 1, fp) == 1) {
        Node *n = (Node*)malloc(sizeof(Node));
        n->b = tmp;
        n->next = head;
        head = n;
        if (tmp.booking_id > maxid) maxid = tmp.booking_id;
    }
    next_booking_id = maxid + 1;
    fclose(fp);
}
void save_bookings() {
    FILE *fp = fopen(BOOKINGS_FILE, "wb");
    if (!fp) {
        printf("Error: could not open file to save bookings.\n");
        return;
    }
    Node *cur = head;
    while (cur) {
        fwrite(&cur->b, sizeof(Booking), 1, fp);
        cur = cur->next;
    }
    fclose(fp);
}

/* Count existing bookings for a given train */
int count_bookings_for_train(int train_id) {
    int cnt = 0;
    Node *cur = head;
    while (cur) {
        if (cur->b.train_id == train_id) cnt++;
        cur = cur->next;
    }
    return cnt;
}

/* Duplicate detection:
   Returns 1 if duplicate exists (same normalized name, same age, same train_id, same class)
*/
int is_duplicate_booking(const Booking *bk) {
    Node *cur = head;
    while (cur) {
        if (cur->b.age == bk->age &&
            cur->b.train_id == bk->train_id &&
            equalstr_nospaces_case(cur->b.passenger_name, bk->passenger_name) &&
            equalstr_nospaces_case(cur->b.travel_class, bk->travel_class)
           ) {
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}

/* Create a text ticket file (always created) */
void write_ticket_text(const Booking *bk) {
    char fname[128];
    snprintf(fname, sizeof(fname), "booking_%d.txt", bk->booking_id);
    FILE *f = fopen(fname, "w");
    if (!f) return;
    fprintf(f, "Booking ID: %d\n", bk->booking_id);
    fprintf(f, "Name: %s\n", bk->passenger_name);
    fprintf(f, "Age: %d\n", bk->age);
    fprintf(f, "Gender: %s\n", bk->gender);
    fprintf(f, "Train ID: %d\n", bk->train_id);
    fprintf(f, "Class: %s\n", bk->travel_class);
    fprintf(f, "Generated: %s", ctime(&(time_t){time(NULL)}));
    fclose(f);
}

#ifdef HAVE_QRENCODE
/* Generate a PBM QR image using libqrencode.
   Output file: booking_<id>_qr.pbm (ASCII PBM format P1)
*/
void generate_qr_pbm_lib(const Booking *bk) {
    char data[512];
    snprintf(data, sizeof(data),
             "BookingID:%d;Name:%s;Age:%d;Train:%d;Class:%s",
             bk->booking_id, bk->passenger_name, bk->age, bk->train_id, bk->travel_class);
    QRcode *q = QRcode_encodeString8bit(data, 0, QR_ECLEVEL_L);
    if (!q) {
        fprintf(stderr, "QR generation failed.\n");
        return;
    }

    int size = q->width;
    char fname[128];
    snprintf(fname, sizeof(fname), "booking_%d_qr.pbm", bk->booking_id);
    FILE *f = fopen(fname, "w");
    if (!f) { QRcode_free(q); return; }

    // Write ASCII PBM (P1)
    fprintf(f, "P1\n%d %d\n", size, size);
    unsigned char *d = q->data;
    for (int y=0;y<size;y++) {
        for (int x=0;x<size;x++) {
            int isblack = d[y*size + x] & 1; // 1 = black
            fprintf(f, "%d ", isblack ? 1 : 0);
        }
        fprintf(f, "\n");
    }
    fclose(f);
    QRcode_free(q);
    printf("QR code (PBM) saved to %s\n", fname);
}
#endif

/* Fallback ASCII "QR-like" generator if libqrencode not available */
void generate_qr_fallback(const Booking *bk) {
    // create a simple deterministic ascii block based on booking id and name hash
    unsigned int h = 5381;
    const char *s = bk->passenger_name;
    while (*s) h = ((h << 5) + h) + (unsigned char)(*s++);
    h ^= (unsigned int)bk->booking_id;
    int dim = 21; // small square
    char fname[128];
    snprintf(fname, sizeof(fname), "booking_%d_qr.txt", bk->booking_id);
    FILE *f = fopen(fname, "w");
    if (!f) return;
    fprintf(f, "ASCII QR placeholder for Booking %d\n\n", bk->booking_id);
    for (int y=0;y<dim;y++) {
        for (int x=0;x<dim;x++) {
            unsigned int val = (h + x*131 + y*137) & 0xFF;
            if (val % 3 == 0) fputc('#', f);
            else fputc(' ', f);
        }
        fputc('\n', f);
    }
    fclose(f);
    printf("ASCII QR placeholder saved to %s (real QR disabled)\n", fname);
}

/* Generate QR (tries libqrencode if available) */
void generate_qr(const Booking *bk) {
    // always write text ticket too
    write_ticket_text(bk);
#ifdef HAVE_QRENCODE
    generate_qr_pbm_lib(bk);
#else
    generate_qr_fallback(bk);
#endif
}

/* Print trains */
void list_trains() {
    printf("\nAvailable Trains:\n");
    printf("ID   Name               From -> To           Seats Avail\n");
    printf("-------------------------------------------------------\n");
    for (int i = 0; i < MAX_TRAINS; ++i) {
        int booked = count_bookings_for_train(trains[i].id);
        int avail = trains[i].total_seats - booked;
        printf("%-4d %-18s %-10s -> %-10s %5d\n",
               trains[i].id,
               trains[i].name,
               trains[i].from,
               trains[i].to,
               avail);
    }
}

/* Book ticket with duplicate check and QR generation */
void book_ticket() {
    Booking bk;
    char temp[256];

    printf("\n--- Book Ticket ---\n");
    printf("Enter passenger name: ");
    fgets(temp, sizeof(temp), stdin); chomp(temp);
    while (strlen(temp) == 0) {
        printf("Name cannot be empty. Enter passenger name: ");
        fgets(temp, sizeof(temp), stdin); chomp(temp);
    }
    strncpy(bk.passenger_name, temp, MAX_NAME);

    printf("Enter age: ");
    if (scanf("%d", &bk.age) != 1) {
        printf("Invalid input. Booking canceled.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n'); // clear the rest

    printf("Enter gender (Male/Female/Other): ");
    fgets(temp, sizeof(temp), stdin); chomp(temp);
    strncpy(bk.gender, temp, sizeof(bk.gender));

    list_trains();
    printf("Enter train ID to book: ");
    if (scanf("%d", &bk.train_id) != 1) {
        printf("Invalid train ID. Booking canceled.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    // validate train id
    int found = 0;
    Train chosenTrain;
    for (int i = 0; i < MAX_TRAINS; ++i) {
        if (trains[i].id == bk.train_id) {
            found = 1;
            chosenTrain = trains[i];
            break;
        }
    }
    if (!found) {
        printf("Train ID not found. Booking canceled.\n");
        return;
    }

    // check availability
    int booked = count_bookings_for_train(bk.train_id);
    if (booked >= chosenTrain.total_seats) {
        printf("Sorry, no seats available on %s.\n", chosenTrain.name);
        return;
    }

    printf("Enter travel class (e.g. Sleeper, AC, 2A): ");
    fgets(temp, sizeof(temp), stdin); chomp(temp);
    strncpy(bk.travel_class, temp, MAX_CLASS);

    // duplicate check
    if (is_duplicate_booking(&bk)) {
        printf("\nDuplicate booking detected! A booking with the same details already exists.\n");
        printf("To prevent fraud, the system will not create a duplicate booking.\n");
        return;
    }

    bk.booking_id = next_booking_id++;
    // insert at head
    Node *n = (Node*)malloc(sizeof(Node));
    n->b = bk;
    n->next = head;
    head = n;

    save_bookings();
    printf("\nBooking successful! Booking ID: %d\n", bk.booking_id);
    printf("Passenger: %s | Train: %s (%s -> %s) | Class: %s\n",
           bk.passenger_name, chosenTrain.name, chosenTrain.from, chosenTrain.to, bk.travel_class);

    // generate QR and ticket file
    generate_qr(&bk);
}

/* View all bookings */
void view_bookings() {
    if (!head) {
        printf("\nNo bookings found.\n");
        return;
    }
    printf("\n--- All Bookings ---\n");
    printf("ID  Name                          Age Gender  Train           Class\n");
    printf("---------------------------------------------------------------------\n");
    Node *cur = head;
    while (cur) {
        // find train name
        char trainname[80] = "Unknown";
        for (int i = 0; i < MAX_TRAINS; ++i) {
            if (trains[i].id == cur->b.train_id) {
                strncpy(trainname, trains[i].name, sizeof(trainname));
                break;
            }
        }
        printf("%-4d %-28s %-3d  %-6s  %-15s %-s\n",
               cur->b.booking_id,
               cur->b.passenger_name,
               cur->b.age,
               cur->b.gender,
               trainname,
               cur->b.travel_class);
        cur = cur->next;
    }
}

/* Search booking by ID */
void search_booking() {
    printf("\nEnter Booking ID to search: ");
    int id;
    if (scanf("%d", &id) != 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    Node *cur = head;
    while (cur) {
        if (cur->b.booking_id == id) {
            // print details
            Train chosenTrain = {0};
            for (int i = 0; i < MAX_TRAINS; ++i) {
                if (trains[i].id == cur->b.train_id) { chosenTrain = trains[i]; break; }
            }
            printf("\nBooking found:\n");
            printf("Booking ID: %d\n", cur->b.booking_id);
            printf("Name: %s\n", cur->b.passenger_name);
            printf("Age: %d\n", cur->b.age);
            printf("Gender: %s\n", cur->b.gender);
            printf("Train: %s (%s -> %s)\n", chosenTrain.name, chosenTrain.from, chosenTrain.to);
            printf("Class: %s\n", cur->b.travel_class);
            return;
        }
        cur = cur->next;
    }
    printf("Booking with ID %d not found.\n", id);
}

/* Cancel booking by ID */
void cancel_booking() {
    printf("\nEnter Booking ID to cancel: ");
    int id;
    if (scanf("%d", &id) != 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    Node *cur = head, *prev = NULL;
    while (cur) {
        if (cur->b.booking_id == id) {
            // remove node
            if (prev) prev->next = cur->next;
            else head = cur->next;
            free(cur);
            save_bookings();
            printf("Booking %d canceled successfully.\n", id);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    printf("Booking ID %d not found.\n", id);
}

/* Free linked list on exit */
void free_all() {
    Node *cur = head;
    while (cur) {
        Node *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    head = NULL;
}

void show_menu() {
    printf("\n================ Railway Ticket Booker ================\n");
    printf("1. List Trains\n");
    printf("2. Book Ticket\n");
    printf("3. View All Bookings\n");
    printf("4. Search Booking by ID\n");
    printf("5. Cancel Booking\n");
    printf("6. Exit\n");
    printf("Enter choice: ");
}

int main() {
    load_bookings();
    int choice;
    while (1) {
        show_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number 1-6.\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');
        switch (choice) {
            case 1: list_trains(); break;
            case 2: book_ticket(); break;
            case 3: view_bookings(); break;
            case 4: search_booking(); break;
            case 5: cancel_booking(); break;
            case 6:
                save_bookings();
                free_all();
                printf("Goodbye!\n");
                exit(0);
            default:
                printf("Invalid choice. Please choose 1-6.\n");
        }
    }
    return 0;
}
