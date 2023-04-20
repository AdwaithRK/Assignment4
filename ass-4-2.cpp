#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

#define MAX_QUESTIONS 10
#define MAX_ANSWERS 5

struct question {
    long type;
    char text[100];
};

struct answer {
    long type;
    int student_id;
    int grades[MAX_QUESTIONS];
};

void student_process(int id, int num_questions, int msgqid_questions, int msgqid_answers) {
    struct question q;
    struct answer a;
    srand(time(NULL) + id);

    for (int i = 0; i < num_questions; i++) {
        msgrcv(msgqid_questions, &q, sizeof(q.text), id, 0);
        for (int j = 0; j < MAX_QUESTIONS; j++) {
            a.grades[j] = (rand() % MAX_ANSWERS) + 1;
        }
        a.type = q.type;
        a.student_id = id;
        msgsnd(msgqid_answers, &a, sizeof(a.grades), 0);
    }
    exit(0);
}

int main() {
    int num_students, num_questions;
    printf("Enter the number of students: ");
    scanf("%d", &num_students);
    printf("Enter the number of questions: ");
    scanf("%d", &num_questions);

    int msgqid_questions = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    int msgqid_answers = msgget(IPC_PRIVATE, IPC_CREAT | 0666);

    for (int i = 1; i <= num_students; i++) {
        if (fork() == 0) {
            student_process(i, num_questions, msgqid_questions, msgqid_answers);
        }
    }

    struct question q;
    struct answer a;
    int grades[num_students][num_questions];
    int total_grades[num_students] = {0};

    for (int i = 0; i < num_questions; i++) {
        q.type = i + 1;
        printf("Enter question %d: ", i + 1);
        scanf(" %[^\n]", q.text);
        for (int j = 1; j <= num_students; j++) {
            msgsnd(msgqid_questions, &q, sizeof(q.text), 0);
        }
    }

    for (int i = 0; i < num_students * num_questions; i++) {
        msgrcv(msgqid_answers, &a, sizeof(a.grades), 0, 0);
        for (int j = 0; j < num_questions; j++) {
            grades[a.student_id - 1][j] = a.grades[j];
            total_grades[a.student_id - 1] += a.grades[j];
        }
    }

    printf("\nGrade distribution:\n");
    for (int i = 0; i < num_students; i++) {
        printf("Student %d: %d/%d\n", i + 1, total_grades[i], MAX_QUESTIONS * MAX_ANSWERS);
    }

    for (int i = 0; i < num_students; i++) {
        wait(NULL);
    }

    msgctl(msgqid_questions, IPC_RMID, NULL);
    msgctl(msgqid_answers, IPC_RMID, NULL);
    return 0;
}