#include <igloo/igloo_alt.h>

#include "../src/observer/ScoreSubject.h"
#include "../src/observer/HudObserver.h"
#include "../src/observer/MilestoneObserver.h"

using namespace igloo;

Describe(ScoreSubjectSpec) {

    It(accumulates_points) {
        ScoreSubject subject;
        subject.addPoints(10);
        subject.addPoints(15);
        Assert::That(subject.score(), Equals(25));
    };

    It(notifies_a_registered_observer) {
        ScoreSubject subject;
        HudObserver hud;
        subject.addObserver(&hud);
        subject.addPoints(30);
        Assert::That(hud.displayed(), Equals(30));
    };

    It(notifies_every_registered_observer) {
        ScoreSubject subject;
        HudObserver a, b;
        subject.addObserver(&a);
        subject.addObserver(&b);
        subject.addPoints(7);
        Assert::That(a.displayed(), Equals(7));
        Assert::That(b.displayed(), Equals(7));
    };

    It(stops_notifying_a_removed_observer) {
        ScoreSubject subject;
        HudObserver hud;
        subject.addObserver(&hud);
        subject.addPoints(10);     // hud sees 10
        subject.removeObserver(&hud);
        subject.addPoints(10);     // hud should not see 20
        Assert::That(hud.displayed(), Equals(10));
    };

    It(reports_its_observer_count) {
        ScoreSubject subject;
        HudObserver hud;
        Assert::That(subject.observerCount(), Equals(0));
        subject.addObserver(&hud);
        Assert::That(subject.observerCount(), Equals(1));
        subject.removeObserver(&hud);
        Assert::That(subject.observerCount(), Equals(0));
    };
};

Describe(MilestoneObserverSpec) {

    It(does_not_fire_before_the_first_milestone) {
        ScoreSubject subject;
        MilestoneObserver milestone(100);
        subject.addObserver(&milestone);
        subject.addPoints(90);
        Assert::That(milestone.consumeJustReached(), Equals(false));
        Assert::That(milestone.milestonesReached(), Equals(0));
    };

    It(fires_when_a_milestone_is_crossed) {
        ScoreSubject subject;
        MilestoneObserver milestone(100);
        subject.addObserver(&milestone);
        subject.addPoints(120);
        Assert::That(milestone.milestonesReached(), Equals(1));
        Assert::That(milestone.consumeJustReached(), Equals(true));
    };

    It(consumes_the_one_shot_flag) {
        ScoreSubject subject;
        MilestoneObserver milestone(100);
        subject.addObserver(&milestone);
        subject.addPoints(100);
        Assert::That(milestone.consumeJustReached(), Equals(true));
        Assert::That(milestone.consumeJustReached(), Equals(false)); // already consumed
    };

    It(counts_multiple_milestones) {
        ScoreSubject subject;
        MilestoneObserver milestone(100);
        subject.addObserver(&milestone);
        subject.addPoints(250); // crosses 100 and 200
        Assert::That(milestone.milestonesReached(), Equals(2));
    };

    It(lets_two_observers_react_to_one_change_differently) {
        ScoreSubject subject;
        HudObserver hud;
        MilestoneObserver milestone(100);
        subject.addObserver(&hud);
        subject.addObserver(&milestone);
        subject.addPoints(100);
        Assert::That(hud.displayed(), Equals(100));         // HUD shows score
        Assert::That(milestone.milestonesReached(), Equals(1)); // milestone fires
    };
};
